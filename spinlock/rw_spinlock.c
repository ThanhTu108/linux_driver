#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//fops
#include <linux/fs.h>
#include <linux/cdev.h>
//sysfs
#include <linux/kobject.h>
#include <linux/sysfs.h>
//kthread
#include <linux/kthread.h>
#include <linux/delay.h>


volatile int sysclass_val = 0;
volatile int rwspinlock_val = 0;
dev_t dev_num = 0;
static struct class* dev_class;
static struct device* dev_file;
static struct cdev my_cdev;
static struct kobject* my_kobj;

//spinlock
// DEFINE_SPINLOCK(name);
DEFINE_RWLOCK(spinlock_rw);
//thread
struct task_struct* thread_1;
struct task_struct* thread_2;

int thread_1_fn(void* pv);
int thread_2_fn(void* pv);

static int __init rw_spinlock(void);
static void __exit rm_rw_spinlock(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
static struct kobj_attribute my_attr = __ATTR(val_rwspinlock, 0660, sys_show, sys_store);

struct file_operations my_fops = 
{
    .owner = THIS_MODULE, 
    .open = open_fops, 
    .read = read_fops,
    .write = write_fops,
    .release = release_fops,
};


static int open_fops(struct inode* inode, struct file* file)
{
    pr_info("OPEN\n");
    return 0;
}

static int release_fops(struct inode* inode, struct file* file)
{
    pr_info("RELEASE\n");
    return 0;
}
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("READ \n");
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("\nSYS_SHOW: READ\n");
    return sprintf(buf, "%d\n", sysclass_val);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("\nSYS_STORE: WRITE\n");
    sscanf(buf, "%d", &sysclass_val);
    return count;
}

int thread_1_fn(void* pv)
{
    while(!kthread_should_stop())
    {
        read_lock(&spinlock_rw);
        pr_info("Thread 1, Read val: %d\n", rwspinlock_val);
        read_unlock(&spinlock_rw);
        msleep(1000);
    }
    return 0;
}
int thread_2_fn(void* pv)
{
    while(!kthread_should_stop())
    {
        write_lock(&spinlock_rw);
        rwspinlock_val++;
        pr_info("Thread 2, Write val: %d\n", rwspinlock_val);
        write_unlock(&spinlock_rw);
        msleep(1000);
    }
    return 0;
}

static int __init rw_spinlock(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "rw_spinlock_num") < 0)
    {
        pr_err("Cannot create major minor number \n");
        return -1;
    }
    pr_info("MAJOR(%d)\nMINOR(%d)", MAJOR(dev_num), MINOR(dev_num));
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add cdev to system\n");
        goto r_class;
    }

    dev_class = class_create(THIS_MODULE, "rw_spinlock_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_num, NULL, "rw_spinlock_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    my_kobj = kobject_create_and_add("sys_rwspinlock", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Create sysfs fail\n");
        goto r_sysfs;
    }
    thread_1 = kthread_run(thread_1_fn, NULL, "thread1");
    if(thread_1)
    {
        pr_info("Thread 1 done\n");
    }
    else
    {
        pr_err("Create treate thread 1 fail\n");
        goto r_device;
    }
    thread_2 = kthread_run(thread_2_fn, NULL, "thread1");
    if(thread_2)
    {
        pr_info("Thread 2 done\n");
    }
    else
    {
        pr_err("Create treate thread 2 fail\n");
        goto r_device;
    }
    pr_info("Insert thread, rw spinlock done \n");
    return 0;
r_sysfs:
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
r_device:
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit rm_rw_spinlock(void)
{
    kthread_stop(thread_1);
    kthread_stop(thread_2);
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("REMOVE DONE \n");
}

module_init(rw_spinlock);
module_exit(rm_rw_spinlock);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
