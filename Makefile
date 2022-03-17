ifeq ($(KERNELRELEASE), )
KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD :=$(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR)  M=$(PWD)  
clean:
	rm -rf *.mk .tmp_versions Module.symvers *.mod.c *.o *.ko .*.cmd Module.markers modules.order
load:
	insmod wdt-test.ko
unload:
	rmmod wdt-test
install: default
	mkdir -p /lib/modules/$(shell uname -r)/kernel/drivers/misc/
	cp -f ./wdt-test.ko /lib/modules/$(shell uname -r)/kernel/drivers/misc/
	depmod -a
	echo "ft8042" >> /etc/modules
uninstall:
	rm -rf /lib/modules/$(shell uname -r)/kernel/drivers/misc/wdt-test.ko
	depmod -a
else
	obj-m := wdt-test.o
endif
