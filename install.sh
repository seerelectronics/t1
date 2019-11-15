apt install linux-modules-extra-$(uname -r)
make
make install
depmod -a

