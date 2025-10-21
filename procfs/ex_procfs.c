#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>

//fops 
#include <linux/cdev.h>
#include <linux/fs.h>

//device file
#include <linux/device.h>
#include <linux/kdev_t.h>

//kmalloc, kfree
#include <linux/slab.h>
//copy_from/to_user
#include <linux/uaccess.h>

#include <linux/ioctl.h>

//procfs
#include <linux/proc_fs.h>

#define LINUX_KERNEL_VERSION  618
dev_t dev_num = 0;
struct class* dev_class;
struct device* dev_file;
struct cdev cdev_ops;

int32_t len_eof = 0;
char kernel_buf[20] = "test proc\n";

//proc
static struct proc_dir_entry* parent;



static int __init create_procfs(void);
static void __exit remove_procfs(void);
static int open_proc(struct inode* inode, struct file* file);
static int release_proc(struct inode* inode, struct file* file);
static ssize_t read_proc(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_proc(struct file* file, const char __user* buf, size_t len, loff_t* off);

static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);

#if (LINUX_KERNEL_VERSION > 505)
static struct proc_ops my_proc = 
{
    .proc_open = open_proc,
    .proc_read = read_proc,
    .proc_write = write_proc,
    .proc_release = release_proc,
};
#else
static struct file_operations my_fops = 
{
        .open = open_fops,
        .read = read_fops,
        .write = write_fops,
        .release = release_fops,
};
#endif

static int open_proc(struct inode* inode, struct file* file)
{
    pr_info("Open proc function\n");
    return 0;
}

static int open_fops(struct inode* inode, struct file* file)
{
    pr_info("Open function\n");
    return 0;
}

static int release_proc(struct inode* inode, struct file* file)
{
    pr_info("Release function\n");
    return 0;
}

static ssize_t read_proc(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    if(len_eof)
    {
        len_eof = 0;
    }
    else
    {
        len_eof = 1;
        return 0;
    }
    if(copy_to_user(buf, kernel_buf, strlen(kernel_buf) + 1))
    {
        pr_err("Data read: Err!!!\n");
    }
    return len;
}
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    pr_info("Read func\n");
    return 0;
}


static ssize_t write_proc(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    memset(kernel_buf, 0, sizeof(kernel_buf));
    if(copy_from_user(kernel_buf, buf, len))
    {
        pr_err("Data write: Err!!!\n");
    }
    pr_info("Data: %s", kernel_buf);
    return len;
}
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("Write function\n");
    return 0;
}

static int __init create_procfs(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1,"procfs_mm") < 0)
    {
        pr_err("Cannot allocate major, minor number\n");
        return -1;
    }
    pr_info("Major(%d)\nMinor(%d)", MAJOR(dev_num), MINOR(dev_num));

    dev_class = class_create(THIS_MODULE, "procfs_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_num, NULL, "procfs_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file");
        goto r_device;
    }
//Don't use in procfs
    // cdev_init(&cdev_ops, &my_fops);
    // if(cdev_add(&cdev_ops, dev_num, 1) < 0)
    // {
    //     pr_err("Cannot add device to system\n");
    //     goto r_class;
    // }

    //create procfs entry
    //proc/ex_procfs
    parent = proc_mkdir("ex_procfs", NULL);
    if(parent == NULL)
    {
        pr_err("Cannot create procfs!!!\n");
        goto r_device;
    }
    if(proc_create("procfs_1", 0666, parent,  &my_proc) == NULL)
    {
        pr_err("Err create proc/ex_procfs/procfs_1\n");
    }

    pr_info("Done proc\n");

    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev_num, 1);
    return -1;
}

static void __exit remove_procfs(void)
{
    proc_remove(parent);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    // cdev_del(&cdev_ops);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove procfs done\n");
}

module_init(create_procfs);
module_exit(remove_procfs);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_DESCRIPTION("simple driver procfs");