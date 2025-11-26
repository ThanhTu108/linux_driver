# Misc device

Misc device is a special character device. 
**What is difference between character device and misc device**
- In misc device the major number is 10 and minor between 1 to 255. It's also have all the file operaion like character device
- Misc device will automatically generated device file (class_create, device_create, cdev_init, cdev_add)

## Function in misc device
To create misc device we use miscdevice struct. After that, we are register the device. Once we have done with the operation, remember unregister the device 
```c
#include <linux/miscdevice.h>
```
Struct misc device: 

```c
struct miscdevice
{
    int minor;
    const char* name;
    struct file_operations* fops,
    struct list_head list;
    struct device* parent;
    struct device* this_device;
    const struct attribute_group** group;
    const char* nodename;
    umode_t mode;
}; 
```
- Where:
    - minor: you can hard code your minor number (check ls -l/dev) if it available, or you can use **MISC_DYNAMIC_MINOR**  
    - name: name of misc device (/dev/name);
    - fops: the pointer to your file_operations
    - group: assign your sysfs attribute
    - nodename: name of sys
    - mode: "0666" rwe for all user

**Register misc device**

Register the misc device with the kernel (init)
```c
int misc_register(struct miscdevice *misc);
```

**un-register misc device**
Register the misc device with the kernel (exit)
```c
void misc_deregister(struct miscdevice *misc);
```
