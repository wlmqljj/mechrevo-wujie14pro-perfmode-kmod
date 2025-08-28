#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/acpi.h>
#include <linux/sysfs.h>
#include <linux/slab.h>

#define DRV_NAME "mechrevo-wujie14pro-perfmode"

#define ACPI_PATH_ECWT "\\_SB.PCI0.LPC0.H_EC.ECWT"
#define ACPI_PATH_ITSM "\\_SB.PCI0.LPC0.H_EC.ITSM"
#define TURBO "turbo"
#define BALANCE "balance"
#define SILENCE "silence"

enum mechrevo_powermode {
    MECHREVO_TURBO = 0,
    MECHREVO_BALANCE = 1,
    MECHREVO_SILENCE = 2,
    MECHREVO_INVALID = -1
};

static const char *mechrevo_powermode_to_str(enum mechrevo_powermode mode)
{
    switch (mode) {
        case MECHREVO_BALANCE: return BALANCE;
        case MECHREVO_TURBO: return TURBO;
        case MECHREVO_SILENCE: return SILENCE;
        default: return "unknown";
    }
}

static enum mechrevo_powermode mechrevo_powermode_from_str(const char *str)
{
    if (strncmp(str, BALANCE, strlen(BALANCE)) == 0) return MECHREVO_BALANCE;
    if (strncmp(str, TURBO,  strlen(TURBO)) == 0) return MECHREVO_TURBO;
    if (strncmp(str, SILENCE, strlen(SILENCE)) == 0) return MECHREVO_SILENCE;
    return MECHREVO_INVALID;
}

// 读取ITSM字段
static int ec_read_itsm(u8 *value)
{
    acpi_handle handle;
    struct acpi_buffer ret = { ACPI_ALLOCATE_BUFFER, NULL };
    union acpi_object *out;
    acpi_status status;

    status = acpi_get_handle(NULL, ACPI_PATH_ITSM, &handle);
    if (ACPI_FAILURE(status)) {
        pr_err(DRV_NAME ": Cannot get ITSM handle\n");
        return -EIO;
    }
    status = acpi_evaluate_object(handle, NULL, NULL, &ret);
    if (ACPI_FAILURE(status)) {
        pr_err(DRV_NAME ": Cannot evaluate ITSM\n");
        return -EIO;
    }
    out = (union acpi_object *)ret.pointer;
    if (out->type != ACPI_TYPE_INTEGER) {
        pr_err(DRV_NAME ": ITSM not integer\n");
        kfree(ret.pointer);
        return -EINVAL;
    }
    *value = (u8)out->integer.value;
    kfree(ret.pointer);
    return 0;
}

// 写入ITSM字段
static int ec_write_itsm(u8 value)
{
    acpi_status status;
    acpi_handle itsm_handle;
    struct acpi_object_list params;
    union acpi_object param_objs[2];

    status = acpi_get_handle(NULL, ACPI_PATH_ITSM, &itsm_handle);
    if (ACPI_FAILURE(status)) {
        pr_err(DRV_NAME ": Cannot get ITSM handle\n");
        return -EIO;
    }

    param_objs[0].type = ACPI_TYPE_INTEGER;
    param_objs[0].integer.value = value;
    param_objs[1].type = ACPI_TYPE_LOCAL_REFERENCE;
    param_objs[1].reference.actual_type = ACPI_TYPE_INTEGER;
    param_objs[1].reference.handle = itsm_handle;

    params.count = 2;
    params.pointer = param_objs;

    status = acpi_evaluate_object(NULL, ACPI_PATH_ECWT, &params, NULL);
    if (ACPI_FAILURE(status)) {
        pr_err(DRV_NAME ": ECWT failed\n");
        return -EIO;
    }
    return 0;
}

// sysfs 节点 show/store 方法
static struct kobject *mechrevo_kobj;

static ssize_t perfmode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    u8 val;
    int ret = ec_read_itsm(&val);
    if (ret) return ret;
    return sysfs_emit(buf, "%s\n", mechrevo_powermode_to_str((enum mechrevo_powermode)val));
}

static ssize_t perfmode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    enum mechrevo_powermode mode = mechrevo_powermode_from_str(buf);
    if (mode == MECHREVO_INVALID) return -EINVAL;
    if (ec_write_itsm((u8)mode)) return -EIO;
    return count;
}

static struct kobj_attribute perfmode_attr = __ATTR(perfmode, 0664, perfmode_show, perfmode_store);

static int __init mechrevo_perfmode_init(void)
{
    int ret;
    mechrevo_kobj = kobject_create_and_add("mechrevo_perfmode", kernel_kobj);
    if (!mechrevo_kobj)
        return -ENOMEM;
    ret = sysfs_create_file(mechrevo_kobj, &perfmode_attr.attr);
    if (ret)
        kobject_put(mechrevo_kobj);
    pr_info(DRV_NAME ": loaded\n");
    return ret;
}

static void __exit mechrevo_perfmode_exit(void)
{
    sysfs_remove_file(mechrevo_kobj, &perfmode_attr.attr);
    kobject_put(mechrevo_kobj);
    pr_info(DRV_NAME ": unloaded\n");
}

module_init(mechrevo_perfmode_init);
module_exit(mechrevo_perfmode_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wlmqljj");
MODULE_DESCRIPTION("Mechrevo WuJie14Pro performance mode sysfs control");
