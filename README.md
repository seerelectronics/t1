The Mothonic T1/USB media converter requires two drivers:

   * lan78xx (USB controller)
   * dp83tc811 (T1 PHY)

These are included in kernel 4.18 and later. If you have an older version it is strongly recommended to upgrade, if you still want to stay you can use this package to build the drivers.

Check kernel version with:

    uname -a

If you need to upgrade see instructions below.

When the drivers are available they will be loaded automatically as soon as the converter is plugged in and the T1 interface is available for use. It is configured in the same way as a normal ethernet interface.


## Ubuntu 18

Drivers are included in linux-modules-extra. If you have upgraded from earlier versions of Ubuntu you may need to install new kernel and modules packages:
    
    sudo apt install linux-image-4.18.0-25-generic linux-modules-extra-4.18.0-25-generic
    
After reboot the T1 interface is available for use.

## Ubuntu 16


Supports kernel 4.15 which contains lan78xx but not dp83tc811. Make sure linux-modules-extra is installed and use the provided driver or build your own using this package.

    sudo apt install linux-image-4.15.0-70-generic linux-modules-extra-4.15.0-70-generic
    wget https://raw.github.com/megahallon/moth/master/ubuntu16-4.15.0-70-generic/dp83tc811.ko  
    sudo cp dp83tc811.ko /lib/modules/4.15.0-70-generic/kernel

After reboot the T1 interface is available for use.

## Raspbian Buster

Install prebuilt dp83tc811.ko module from this repository or build using instructions at:

    https://www.raspberrypi.org/documentation/linux/kernel/building.md

### Raspberry 3:

Install prebuilt dp83tc811.ko for the kernel version you are using:

    wget https://raw.github.com/megahallon/moth/master/rp3_4.19.75/dp83tc811.ko
    sudo cp dp83tc811.ko /lib/modules/4.19.75-v7/extra
    sudo depmod -a

### Raspberry 4:

Install prebuilt dp83tc811.ko for the kernel version you are using:

    wget https://raw.github.com/megahallon/moth/master/rp4_4.19.75/dp83tc811.ko
    sudo cp dp83tc811.ko /lib/modules/4.19.75-v7+/extra
    sudo depmod -a
