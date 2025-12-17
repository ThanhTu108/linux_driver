.PHONY: all clean

obj-m :=project.o
ssd_driver-objs :=prj.o ssd1306.o font5x7.o
ARCH = arm
KDIR:=/home/tu/BBG/tmp/lib/modules/6.1.83/build
CROSS_COMPILE = arm-linux-gnueabi-
all:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	make -C $(KDIR) M=$(shell pwd) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) clean