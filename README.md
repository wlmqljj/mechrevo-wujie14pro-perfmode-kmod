# mechrevo-wujie14pro-perfmode-kmod
Mechrevo WuJie14Pro performance mode sysfs control kernel module  
Tested under kernel 6.12-LTS  
## Introduction
A tiny kernel module exposed as sysFs interface to interact with Mechrevo WuJie 14Pro performance mode flag within EC.  
   
Kernel sysFs interface:  
```  
/sys/kernel/mechrevo_perfmode/perfmode
```  
  
the attribute have show/store method  
  
The perfmode attribute allow these value:  
```
turbo balance silence
```  
You can read/change the attr by:
```
cat /sys/kernel/mechrevo_perfmode/perfmode
echo 'balance' | sudo tee /sys/kernel/mechrevo_perfmode/perfmode
```
## Installation
### Archlinux:
By executing under working directory:
```
makepkg -si
```
## Udev auto perfmode changing
Also, there is an example `99-mechrevo-perfmode.rules` udev rule file can be put under
`/etc/udev/rules.d/`
to change the perfmode automatically between turbo and silence mode when power supply state changed.

After coping the udev file, you can execute
```
sudo udevadm control --reload
sudo udevadm trigger --subsystem-match=power_supply
```
to make udevd take the added rule into effect.  
