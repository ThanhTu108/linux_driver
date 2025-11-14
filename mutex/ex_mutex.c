#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
//file operations
#include <linux/fs.h>
#include <linux/cdev.h>
//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//k thread
#include <linux/kthread.h>  //kernel thread
#include <linux/sched.h>    //task struct
//sysfs
#include <linux/kobject.h>
#include <linux/sysfs.h>
//mutex
#include <linux/mutex.h>
#include <linux/delay.h>

volatile unsigned long val_test_mutex = 0; 
dev_t dev_num = 0;
static struct class* dev_class;
static struct device* dev_file;
static struct kobject* my_kobj;
static struct cdev my_cdev;

//thread
static struct task_struct* thread_1;
static struct task_struct* thread_2;
//thread function
int thread_1_fn(void* pv);
int thread_2_fn(void* pv);
//mutex
static struct mutex my_mutex;

//function prototypes
static int __init add_mutex_ex(void);
static void __exit remove_mutex_ex(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);
static ssize_t sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count);
static struct kobj_attribute my_attr = __ATTR(val_mutex_ex, 0660, sys_show, sys_store);

//struct fops
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
    pr_info("SYSFS_SHOW: read\n");
    return 0;
}
static ssize_t sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    pr_info("SYSFS_STORE: write\n");
    return count;
}
//thread_func
int thread_1_fn(void* pv)
{
    pr_info("Thread 1: \n");
    while(!kthread_should_stop())
    {
        mutex_lock(&my_mutex);
        val_test_mutex++;
        pr_info("Value of thread 1: %lu\n", val_test_mutex);
        mutex_unlock(&my_mutex);
        msleep(1000);
    }
    return 0;
}
int thread_2_fn(void* pv)
{
    pr_info("Thread 2: \n");
    while(!kthread_should_stop())
    {
        mutex_lock(&my_mutex);
        val_test_mutex++;
        pr_info("Value of thread 2: %lu\n", val_test_mutex);
        mutex_unlock(&my_mutex);
        msleep(500);
    }
    return 0;
}
static int __init add_mutex_ex(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "mutex_mm_num") < 0)
    {
        pr_err("Cannot create major and minor\n");
        return -1;
    }
    pr_info("MAJOR(%d)\nMINOR(%d)\n", MAJOR(dev_num), MINOR(dev_num));
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    dev_class= class_create(THIS_MODULE, "mutex_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    dev_file= device_create(dev_class, NULL, dev_num, NULL, "mutex_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create struct class\n");
        goto r_device;
    }
    my_kobj = kobject_create_and_add("sys_mutex", kernel_kobj);
    if(sysfs_create_file(my_kobj, &my_attr.attr))
    {
        pr_err("Cannot create sysfs file\n");
        goto r_sysfs;
    }
    thread_1 = kthread_run(thread_1_fn, NULL, "thread_1");
    if(IS_ERR(thread_1))
    {
        pr_err("Create thread 1 error\n");
        goto r_device;
    }

    thread_2 = kthread_run(thread_2_fn, NULL, "thread_1");
    if(IS_ERR(thread_2))
    {
        pr_err("Create thread 2 error\n");
        goto r_device;
    }
    pr_info("INSERT donr\n");
    return 0;
r_sysfs:
    sysfs_remove_file(my_kobj, &my_attr.attr);
    kobject_put(my_kobj);
r_device:
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit remove_mutex_ex(void)
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

module_init(add_mutex_ex);
module_exit(remove_mutex_ex);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");