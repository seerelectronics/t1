# Needed on Ubuntu 18
#ccflags-y += -fno-pie

KERNELRELEASE ?= $(shell uname -r)
KDIR ?= /lib/modules/$(KERNELRELEASE)/build

obj-m := dp83tc811.o
#lan78xx.o 
#lan78xx-y := src/lan78xx.o
dp83tc811-y := src/dp83tc811.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD)

clean:
	-rm *.o *.ko *.mod.c modules.order Module.symvers

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
