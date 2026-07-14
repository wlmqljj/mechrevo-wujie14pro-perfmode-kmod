// SPDX-License-Identifier: GPL-2.0

#include <linux/acpi.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#define DRV_NAME "mechrevo-wujie14pro-perfmode"

#define ACPI_PATH_ECWT "\\_SB.PCI0.LPC0.H_EC.ECWT"
#define ACPI_PATH_ITSM "\\_SB.PCI0.LPC0.H_EC.ITSM"

#define TURBO "turbo"
#define BALANCE "balance"
#define SILENCE "silence"

enum mechrevo_powermode
{
    MECHREVO_TURBO = 0,
    MECHREVO_BALANCE = 1,
    MECHREVO_SILENCE = 2
};

static const char*
mechrevo_powermode_to_str(enum mechrevo_powermode mode)
{
    switch (mode) {

        case MECHREVO_TURBO:
            return TURBO;

        case MECHREVO_BALANCE:
            return BALANCE;

        case MECHREVO_SILENCE:
            return SILENCE;

        default:
            return "unknown";
    }
}

static int
mechrevo_powermode_from_str(const char* str)
{
    if (sysfs_streq(str, TURBO))
        return MECHREVO_TURBO;

    if (sysfs_streq(str, BALANCE))
        return MECHREVO_BALANCE;

    if (sysfs_streq(str, SILENCE))
        return MECHREVO_SILENCE;

    return -EINVAL;
}

/*
 * Cached ACPI handles
 */

static acpi_handle itsm_handle;
static acpi_handle ecwt_handle;

/*
 * Read ITSM field
 */

static int
ec_read_itsm(u8* value)
{
    struct acpi_buffer ret = {
        .length = ACPI_ALLOCATE_BUFFER,
        .pointer = NULL,
    };

    union acpi_object* out;

    acpi_status status;

    if (!value)
        return -EINVAL;

    if (!itsm_handle) {

        status = acpi_get_handle(NULL, ACPI_PATH_ITSM, &itsm_handle);

        if (ACPI_FAILURE(status)) {

            pr_err(DRV_NAME ": Cannot get ITSM handle\n");

            return -ENODEV;
        }
    }

    status = acpi_evaluate_object(itsm_handle, NULL, NULL, &ret);

    if (ACPI_FAILURE(status)) {

        pr_err(DRV_NAME ": Cannot evaluate ITSM\n");

        return -EIO;
    }

    out = ret.pointer;

    if (!out || out->type != ACPI_TYPE_INTEGER) {

        pr_err(DRV_NAME ": ITSM not integer\n");

        ACPI_FREE(ret.pointer);

        return -EINVAL;
    }

    if (out->integer.value > MECHREVO_SILENCE) {

        pr_err(DRV_NAME ": invalid ITSM value %llu\n", out->integer.value);

        ACPI_FREE(ret.pointer);

        return -EINVAL;
    }

    *value = (u8)out->integer.value;

    ACPI_FREE(ret.pointer);

    return 0;
}

/*
 * Write ITSM through ECWT
 */

static int
ec_write_itsm(u8 value)
{
    struct acpi_object_list params;

    union acpi_object param_objs[2] = {};

    acpi_status status;

    if (value > MECHREVO_SILENCE)
        return -EINVAL;

    if (!itsm_handle) {

        status = acpi_get_handle(NULL, ACPI_PATH_ITSM, &itsm_handle);

        if (ACPI_FAILURE(status))
            return -ENODEV;
    }

    if (!ecwt_handle) {

        status = acpi_get_handle(NULL, ACPI_PATH_ECWT, &ecwt_handle);

        if (ACPI_FAILURE(status))
            return -ENODEV;
    }

    /*
     * Argument 0:
     *
     * New mode value
     */

    param_objs[0].type = ACPI_TYPE_INTEGER;

    param_objs[0].integer.value = value;

    /*
     * Argument 1:
     *
     * Reference(ITSM)
     */

    param_objs[1].type = ACPI_TYPE_LOCAL_REFERENCE;

    param_objs[1].reference.actual_type = ACPI_TYPE_INTEGER;

    param_objs[1].reference.handle = itsm_handle;

    params.count = ARRAY_SIZE(param_objs);

    params.pointer = param_objs;

    status = acpi_evaluate_object(ecwt_handle, NULL, &params, NULL);

    if (ACPI_FAILURE(status)) {

        pr_err(DRV_NAME ": ECWT failed: %s\n", acpi_format_exception(status));

        return -EIO;
    }

    return 0;
}

/*
 * sysfs
 */

static struct kobject* mechrevo_kobj;

static ssize_t
perfmode_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    u8 value;

    int ret;

    ret = ec_read_itsm(&value);

    if (ret)
        return ret;

    return sysfs_emit(
        buf, "%s\n", mechrevo_powermode_to_str((enum mechrevo_powermode)value));
}

static ssize_t
perfmode_store(struct kobject* kobj,
               struct kobj_attribute* attr,
               const char* buf,
               size_t count)
{
    int mode;

    mode = mechrevo_powermode_from_str(buf);

    if (mode < 0)
        return mode;

    if (ec_write_itsm((u8)mode))
        return -EIO;

    return count;
}

static struct kobj_attribute perfmode_attr =
__ATTR(perfmode, 0664, perfmode_show, perfmode_store);

static int __init
mechrevo_perfmode_init(void)
{
    int ret;

    ret = acpi_get_handle(NULL, ACPI_PATH_ITSM, &itsm_handle);

    if (ACPI_FAILURE(ret)) {

        pr_err(DRV_NAME ": ITSM unavailable\n");

        return -ENODEV;
    }

    ret = acpi_get_handle(NULL, ACPI_PATH_ECWT, &ecwt_handle);

    if (ACPI_FAILURE(ret)) {

        pr_err(DRV_NAME ": ECWT unavailable\n");

        return -ENODEV;
    }

    mechrevo_kobj = kobject_create_and_add("mechrevo_perfmode", kernel_kobj);

    if (!mechrevo_kobj)
        return -ENOMEM;

    ret = sysfs_create_file(mechrevo_kobj, &perfmode_attr.attr);

    if (ret) {

        kobject_put(mechrevo_kobj);

        mechrevo_kobj = NULL;

        return ret;
    }

    pr_info(DRV_NAME ": loaded\n");

    return 0;
}

static void __exit
mechrevo_perfmode_exit(void)
{
    if (mechrevo_kobj) {

        sysfs_remove_file(mechrevo_kobj, &perfmode_attr.attr);

        kobject_put(mechrevo_kobj);
    }

    pr_info(DRV_NAME ": unloaded\n");
}

module_init(mechrevo_perfmode_init);
module_exit(mechrevo_perfmode_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wlmqljj");
MODULE_DESCRIPTION("Mechrevo WuJie14Pro performance mode sysfs control");
MODULE_VERSION("1.1");
