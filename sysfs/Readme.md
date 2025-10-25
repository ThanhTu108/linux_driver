# SYSFS 
Sysfs is the virtual filesystem export by linux kernel. It's mounter at /sys directory.

# Create sysfs file in kernel module
To create sysfs file in kernel module we need to use kobject and kobj_attribute structure. <br>

## Kobject

Kobject is the glue that binds the sysfs and kernel objects, define in <linux/kobject.h>. It defines the directory in sysfs. <br>
```c
struct kobject 
{
    char *k_name;
    char name[KOBJ_NAME_LEN];
    struct kref kref;
    struct list_head entry;
    struct kobject *parent;
    struct kset *kset;
    struct kobj_type *ktype;
    struct dentry *dentry;
};
```
name: name of the kobject <br>
parent: parent kobject, when we create a directory in sysfs, it will create under parent directory <br>

## kobj_attribute 
kobj_attribute structure define the sysfs file attribute, define in <linux/kobject.h>. <br>
```c
struct kobj_attribute
{
    struct attribute attr;
    ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);
};
```
attr: attribute structure that define the file name and permission <br>
show: function pointer to read the file (read from kerner and show in user)<br> 
store: function pointer to write the file (write from user and store in kernel)<br>

## Create sysfs file in kernel module
To create sysfs file in kernel module, we need to follow these steps: <br>
1. Create directory in sysfs <br>
```c
struct kobject *kobj_sysfs; // kobject structure
kobj_sysfs = kobject_create_and_add("name_dir", parent); // create directory in /sys/parent/name_dir
```
if parent:
- kernel_kobj, the directory will be created in /sys/kernel/name_dir <br>
- firmware_kobj, the directory will be created in /sys/firmware/name_dir <br>
- NULL, the directory will be created in /sys/name_dir <br>
2. Define attribute structure <br>
```c
struct kobj_attribute name_attr = __ATTR(name_file, 0666, your_show_function, your_store_function);
``` 
3. Create sysfs file <br>
```c
sysfs_create_file(kobj_sysfs, &name_attr.attr); // create sysfs file
```
4. Remove sysfs file and directory in module exit function <br>
```c
sysfs_remove_file(kobj_sysfs, &name_attr.attr); // remove sysfs file
kobject_put(kobj_sysfs); // remove directory
```

## Usage

## Usage
1. Compile the kernel module using `sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-`.
2. Load the module into the kernel using `sudo insmod ex_sysfs.ko`.
3. Check /sys/kernel/ex_sysfs/ to see the created sysfs file.
4. sudo cat /sys/kernel/ex_sysfs/val_sysfs to read the value from sysfs file.
5. sudo echo "new_value" > /sys/kernel/ex_sysfs/val_sysfs
6. Check the kernel log to see the output of the module using `dmesg`.
7. Unload the module from the kernel using `sudo rmmod ex_sysfs`.

