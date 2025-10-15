# MAJOR AND MINOR NUMBER
====================
This is a simple example of how to use major and minor number in a linux kernel device driver.
There are two solutions to allocate major and minor number:

The dev_t type in the <linux/types.h> header file is used to represent device numbers in the Linux kernel.
It is a 32-bit integer that encodes both the major and minor numbers of a device

## Structure of dev_t
MKDEV(int major, int minor): This macro is used to create a dev_t value from the given major and minor numbers.
MAJOR(dev_t dev)
MINOR(dev_t dev)
These macros are used to extract the major and minor numbers from a dev_t value.


## Features
1. Dynamic allocation of major and minor number using alloc_chrdev_region() function.
2. Static allocation of major and minor number using register_chrdev_region() function.

Major and minor number are used to identify a device driver and a specific device respectively.
You can see it in the /proc/devices.
EX: cat /proc/devices | grep <driver_name>

## Usage
1. Compile the kernel module using `sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-`.
2. Load the module into the kernel using `sudo insmod dynamicMM_number.ko` or `sudo insmod staticMM_number.ko`.
3. Check the kernel log to see the output of the module using `dmesg`.

4. Check the /proc/devices to see the major and minor number of the device driver using `cat /proc/devices | grep <driver_name>`.
5. Unload the module from the kernel using `sudo rmmod dynamicMM_number` or `sudo rmmod staticMM_number`.