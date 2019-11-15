The Mothonic T1/USB media converter requires two drivers:

lan78xx (USB controller)
dp83tc811 (T1 PHY)

These are included in kernel 4.18 and later. If using older kernels this package may be of interest.

Ubuntu 18:
Supports kernel 4.18. Make sure linux-modules-extra is installed.

Ubuntu 16:
Supports kernel 4.16 which contains lan78xx but not dp83tc811. Make sure linux-modules-extra is installed.
