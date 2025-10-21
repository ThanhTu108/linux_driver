# IOCTL (Input Output Control) Driver

## Overview
The ioctl driver provides a mechanism for user-space applications to communicate with kernel-space drivers. It allows for sending control commands and data between user applications and device drivers.

## Functionality
1. Create ioctl commands in the driver
- Define ioctl command 
```c
#define "ioctl name" _IOX("magic number", "command number", datatype)
```
“IO“: an ioctl with no parameters <br>
“IOW“: an ioctl with write parameters (copy_from_user) <br>
“IOR“: an ioctl with read parameters (copy_to_user) <br>
“IOWR“: an ioctl with both write and read parameters <br>

- Add the header file <linux/ioctl.h>
2. Implement the ioctl function in the driver
```c
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd)
    {
        case "ioctl name(read)":
            copy_to_user((datatype*)arg, &kernel_variable, sizeof(datatype));
            // Handle read ioctl command
            break;
        default:
            return -ENOTTY; // Command not supported
    }
}
```
3. Create ioctl commands in user application
```c
#define "ioctl name" _IOX("magic number", "command number", datatype)
```
4. Use ioctl in user application
```c
ioctl(fd, ioctl_name, (datatype*) &number); 
```


## Usage
1. Compile the kernel module using `sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-`.
2. Load the module into the kernel using `sudo insmod ex_ioctl.ko`.
3. Check device file create in ls -l /dev/ioctl_device.

4. Compile the user space application using arm-linux-gnueabi-gcc ioctl_user.c -o app -static
5. Run the user space application using `sudo ./app`.
6. Check the kernel log to see the output of the module using `dmesg`.
7. Unload the module from the kernel using `sudo rmmod ex_ioctl`.