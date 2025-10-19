# File operation

This is a simple linux kernel module that demonstrates how to use file operation in character device driver.

## Flow of file operation driver
1. Define file operation functions (open, read, write, release)
2. Create and initialize file_operations structure
3. Allocate and initialize cdev structure. Using(cdev_init and cdev_add)
4. Register character device with the system
5. Implement module init and exit functions

## Usage
1. cd test_fops.c
2. sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
3. sudo insmod test_fops.ko
4. sudo su; echo "Hello World" > /dev/device_fops
5. check kernel log: dmesg
6. sudo rmmod test_fops