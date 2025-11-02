# obj-m +=ex_interrupt.o 

# ifdef ARCH
# 	KDIR := /home/tu/BBG/tmp/lib/modules/6.1.83/build
# else
# 	KDIR = /lib/modules/$(shell uname -r)/build
# endif

# all:
# 	make -C $(KDIR) M=$(shell pwd) modules
# clean:
# 	make -C $(KDIR) M=$(shell pwd) clean 



obj-m += ex_interrupt.o
ARCH = arm
KDIR := /home/tu/BBG/tmp/lib/modules/6.1.83/build
# KDIR = /lib/modules/$(shell uname -r)/build
CROSS_COMPILE = arm-linux-gnueabi-
all:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
# 	make -C $(KDIR) M=$(shell pwd) modules
clean:
	make -C $(KDIR) M=$(shell pwd) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) clean
# 	make -C $(KDIR) M=$(shell pwd) clean