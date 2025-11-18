.PHONY: all clean

obj-m +=send_signal.o

ARCH = arm
KDIR:=/home/tu/BBG/tmp/lib/modules/6.1.83/build
CROSS_COMPILE = arm-linux-gnueabi-
all:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	make -C $(KDIR) M=$(shell pwd) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) clean