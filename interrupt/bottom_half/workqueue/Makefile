.PHONY: all clean

obj-m:=static_workqueue.o
obj-m+=dynamic_workqueue.o
obj-m+=owner_workqueue.o
KDIR:= /home/tu/BBG/tmp/lib/modules/6.1.83/build
ARCH:= arm
CROSS_COMPILE:= arm-linux-gnueabi-

all:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean