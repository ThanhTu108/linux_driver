.PHONY: all clean module dt
obj-m +=test_probe.o
ARCH = arm
KDIR := /home/tu/BBG/tmp/lib/modules/6.1.83/build
CROSS_COMPILE = arm-linux-gnueabi-
all: module

module:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	make -C $(KDIR) M=$(shell pwd) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) clean