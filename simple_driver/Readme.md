# Simple Driver
This is a simple driver module for demonstration purposes.

## Introduction
This driver provides basic read and write functionalities to interact with user space applications.
Functions uses in this driver 

Functions used in this driver include:
- kmalloc(): Allocates memory in kernel space.
- kfree(): Frees allocated memory in kernel space.
```c
#include <linux/slab.h>
kmalloc(size_t size, gfp_t flags);
```
flags:
- GFP_USER: allocate memory on behalf of a user. May sleep.
- GFP_KERNEL: allocate normal kernel ram. May sleep
- GFP_ATOMIC: allocation will not sleep. May use emergency pools. For example, use this inside interrupt handler.
To learn more about flags, refer to <a href="https://embetronicx.com/tutorials/linux/device-drivers/linux-device-driver-tutorial-programming/">Real driver</a>
```c
#include <linux/slab.h>
kfree(const void *ptr)
```

#include <linux/uaccess.h>
- copy_to_user(): Copies data from kernel space to user space.
- copy_from_user(): Copies data from user space to kernel space.

```c
#include <linux/uaccess.h>
copy_to_user(void __user* to, const void* from, unsigned long n);
```
to: destination address in user space <br>
from: source address in kernel space  <br>
n: number of bytes to copy <br>
```c
#include <linux/uaccess.h>
copy_from_user(void* to, const void __user* from, unsigned long n);
```
to: destination address in kernel space <br>
from: source address in user space <br>
n: number of bytes to copy <br>

## Usage
1. Compile the kernel module using `sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-`.
2. Load the module into the kernel using `sudo insmod simple_driver.ko`.
3. Check device file create in ls -l /dev/simple_driver_df.

4. Compile the user space application using arm-linux-gnueabi-gcc simple_driver_user.c -o app -static
5. Run the user space application using `sudo ./app`.
6. Check the kernel log to see the output of the module using `dmesg`.
7. Unload the module from the kernel using `sudo rmmod simple_driver`.