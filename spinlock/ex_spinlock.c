#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//file operations
#include <linux/cdev.h>
#include <linux/fs.h>
//thread
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>

//spinlock
#include <linux/spinlock.h>

volatile int val_sys = 0;
volatile int val_spinlock;
dev_t dev_num;
static struct class* dev_class;
static struct device* dev_file;
static struct cdev my_cdev;
static struct kobject* my_kobj;

//thread
static struct task_struct* thread_1;
static struct task_struct* thread_2;
//spinlock
// static struct spinlock_t ex_spinlock;
DEFINE_SPINLOCK(ex_spinlock);
//function thread
int thread_1_fn(void* pv);
int thread_2_fn(void* pv);

static int __init create_spinlock_ex(void);
static void __exit remove_spinlock_ex(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
struct kobj_attribute my_attr = __ATTR(val_sys_spin_lock, 0660, sys_show, sys_store);

struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .read = read_fops,
    .write = write_fops,
    .open = open_fops,
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
    pr_info("READ\n");
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}

static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    pr_info("SYS_SHOW: READ \n");
    return sprintf(buf, "%d\n", val_sys);
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("SYS_STORE: WRITE \n");
    sscanf(buf, "%d", &val_sys);
    return count;
}

int thread_1_fn(void* pv)
{
    while(!kthread_should_stop())
    {
        spin_lock(&ex_spinlock);
        val_spinlock++;
        pr_info("Thread 1: %d\n", val_spinlock);
        spin_unlock(&ex_spinlock);
        msleep(1000);
    }
    return 0;
}
int thread_2_fn(void* pv)
{
    while(!kthread_should_stop())
    {
        spin_lock(&ex_spinlock);
        val_spinlock++;
        pr_info("Thread 2: %d\n", val_spinlock);
        spin_unlock(&ex_spinlock);
        msleep(1000);
    }
    return 0;
}

static int __init create_spinlock_ex(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "spinlock_mm_num") < 0)
    {
        pr_err("Cannot create major and minor number\n");
        return -1;
    }
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("cannot add device to system\n");
        goto r_class;
    }

    dev_class = class_create(THIS_MODULE, "spinlock_mm_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_num, NULL, "spinlock_mm_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    thread_1 = kthread_run(thread_1_fn, NULL, "thread_1");
    if(thread_1)
    {
        pr_info("Thread 1 successful\n");
    }
    else
    {
        pr_err("Create thread 1 fail\n");
        goto r_device;
    }

    // Or you can use this method to run your thread
    // thread_1 = kthread_run(thread_1_fn, NULL, "thread_1");
    // if(thread_1)
    // {
    //     wake_up_process(thread_1);
    // }
    // else
    // {
    //     pr_err("Create thread 1 fail\n");
    //     goto r_device;
    // }
    thread_2 = kthread_run(thread_2_fn, NULL, "thread_2");
    if(thread_2)
    {
        pr_info("Thread 2 successful\n");
    }
    else
    {
        pr_err("Create thread 2 fail\n");
        goto r_device;
    }
    my_kobj = kobject_create_and_add("val_sys_spin_lock", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Cannot create sysfs file\n");
        goto r_sysfs;
    }
    pr_info("Insert ex spinlock done\n");
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
static void __exit remove_spinlock_ex(void)
{
    kthread_stop(thread_1);
    kthread_stop(thread_2);
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove done\n");
}

module_init(create_spinlock_ex);
module_exit(remove_spinlock_ex);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");