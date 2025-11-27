#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/uaccess.h>  // copy_to_user, copy_from_user
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/gpio/consumer.h>

#define GPIO_60 60  // số GPIO

// global variables
dev_t dev_num;
static struct class *dev_class;
static struct cdev my_cdev;

// descriptor GPIO
static struct gpio_desc *my_gpio_60;

// file operations prototypes
static int open_fops(struct inode *inode, struct file *file);
static int release_fops(struct inode *inode, struct file *file);
static ssize_t read_fops(struct file *file, char __user *user_buf, size_t len, loff_t *off);
static ssize_t write_fops(struct file *file, const char __user *user_buf, size_t len, loff_t *off);

// file operations
static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = open_fops,
    .release = release_fops,
    .read = read_fops,
    .write = write_fops,
};

static int open_fops(struct inode *inode, struct file *file)
{
    pr_info("GPIO device opened\n");
    return 0;
}

static int release_fops(struct inode *inode, struct file *file)
{
    pr_info("GPIO device closed\n");
    return 0;
}

static ssize_t read_fops(struct file *file, char __user *user_buf, size_t len, loff_t *off)
{
    uint8_t value = gpiod_get_value(my_gpio_60);
    if (copy_to_user(user_buf, &value, 1)) {
        pr_err("Failed to copy GPIO value to user\n");
        return -EFAULT;
    }
    return 1; // trả về 1 byte
}

static ssize_t write_fops(struct file *file, const char __user *user_buf, size_t len, loff_t *off)
{
    char buf[2] = {0};
    if (len > 1) len = 1;
    if (copy_from_user(buf, user_buf, len)) {
        pr_err("Failed to copy data from user\n");
        return -EFAULT;
    }

    if (buf[0] == '1')
        gpiod_set_value(my_gpio_60, 1);
    else if (buf[0] == '0')
        gpiod_set_value(my_gpio_60, 0);
    else
        pr_err("Invalid value, use '0' or '1'\n");

    return len;
}

// init
static int __init gpiod_driver_init(void)
{
    int ret;

    // allocate char device
    ret = alloc_chrdev_region(&dev_num, 0, 1, "gpiod60_dev");
    if (ret < 0) {
        pr_err("Failed to allocate char device\n");
        return ret;
    }

    cdev_init(&my_cdev, &my_fops);
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        pr_err("Failed to add cdev\n");
        goto unregister_chrdev;
    }

    dev_class = class_create(THIS_MODULE, "gpiod60_class");
    if (IS_ERR(dev_class)) {
        pr_err("Failed to create class\n");
        ret = PTR_ERR(dev_class);
        goto del_cdev;
    }

    if (IS_ERR(device_create(dev_class, NULL, dev_num, NULL, "gpiod60"))) {
        pr_err("Failed to create device file\n");
        ret = -1;
        goto destroy_class;
    }

    // lấy descriptor GPIO 60
    my_gpio_60 = gpiod_get(NULL, "gpio_60_debug", GPIOD_OUT_LOW);
    if (IS_ERR(my_gpio_60)) {
        pr_err("Failed to get GPIO 60 descriptor\n");
        ret = PTR_ERR(my_gpio_60);
        goto destroy_device;
    }

    gpiod_export(my_gpio_60, false); // export sysfs nếu muốn debug
    pr_info("GPIO 60 driver initialized\n");
    return 0;

destroy_device:
    device_destroy(dev_class, dev_num);
destroy_class:
    class_destroy(dev_class);
del_cdev:
    cdev_del(&my_cdev);
unregister_chrdev:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

// exit
static void __exit gpiod_driver_exit(void)
{
    gpiod_set_value(my_gpio_60, 0); // tắt LED trước khi free
    gpiod_unexport(my_gpio_60);
    gpiod_put(my_gpio_60);

    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);

    pr_info("GPIO 60 driver removed\n");
}

module_init(gpiod_driver_init);
module_exit(gpiod_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thanh Tu");
MODULE_DESCRIPTION("Char device driver to control GPIO 60 using gpiod API");
