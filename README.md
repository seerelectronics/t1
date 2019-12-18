The Mothonic T1/USB media converter requires two drivers:

   * lan78xx (USB controller)
   * dp83tc811 (T1 PHY)

These are included in kernel 4.18 and later. If you have an older version it is strongly recommended to upgrade, if you still want to stay you can use this package to build the drivers.

Ubuntu 18:
Drivers are included in linux-modules-extra. If you have upgraded from earlier versions of Ubuntu you may need to upgrade the kernel and modules packages.

Check kernel version with:

    uname -a
    
If the version is lower than 4.18 you need to upgrade run the following command:

    sudo apt install linux-image-4.18.0-25-generic linux-modules-extra-4.18.0-25-generic


Ubuntu 16:
Supports kernel 4.16 which contains lan78xx but not dp83tc811. Make sure linux-modules-extra is installed and build dp83tc811 using this package.
