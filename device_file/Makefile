obj-m += static_devicefile.o
obj-m += Auto_create_devfile.o

ifndef
	KDIR := /home/tu/BBG/tmp/lib/modules/6.1.83/build
else
	KDIR = /lib/modules/$(shell uname -r)/build
endif

all:
	make -C ${KDIR} M=$(shell pwd) modules
clean:
	make -C ${KDIR} M=$(shell pwd) clean