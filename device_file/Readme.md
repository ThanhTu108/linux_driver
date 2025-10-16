# Device File Driver
The device file allows transparent communication between user-space applications and hardware.

All device files are located in the '/dev/<device_name>' directory.

ls -l /dev/

## Creating a Device File
### Static Device File
Before creating a device file, ensure you create the major and minor numbers in the kernel source code, then compile and insmod the kernel module.
To create a static device file, use the `mknod` command with the appropriate parameters
```bash
mknod -m <device_name> <type> <major_number> <minor_number>
```
Example:
```bash
mknod -m 666 /dev/my_device c 240 0
```

### Dynamic Device File
Create a major and minor number first, You must to follow three steps:
1. Include header files
```c
#include <linux/cdev.h>
#include <linux/device.h>
```
2. Create a struct Class
```c
static struct class* <your_class>;
your_class = class_create(ThisModule, "your_class_name");
```
3. Create a device file in the init function
```c
static struct device* <your_device>;
your_device = device_create(your_class, NULL, 0(major_number, minor_number), "your_device_name");
```

## Usage
### Automatic Creation
To use example device file

```bash
cd Auto_create_devfile
sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
sudo insmod device_file.ko
```bash
Check the major and minor number in cat /proc/devices | grep auto_create_devfile

auro_create_devfile is my major number

See your device file in ls -l /dev/dev_file
```
### Static Creationde
```bash
cd static_devicefile.c
sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
sudo insmod static_devicefile.ko
```
Check the major and minor number in cat /proc/devices | grep static_device_mm

static_device_mm is my major number

create a device file with mknod command
```bash
sudo mknod -m 666 /dev/static_device_mm c <major_number>


