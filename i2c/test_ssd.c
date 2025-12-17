#include <linux/init.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/property.h>
#include <linux/mod_devicetable.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include "ssd1306.h"
#include "font5x7.h"
static int ssd_probe(struct i2c_client* client, const struct i2c_device_id* id);
static void ssd_remove(struct i2c_client* client);
int count = 0;
static struct of_device_id ssd1306_driver_id[] = 
{
    {
        .compatible = "solomon,ssd1306",
    }, 
    {},
};

MODULE_DEVICE_TABLE(of, ssd1306_driver_id);
static struct i2c_device_id ssd1306_id[] = 
{
    {
        "ssd1306",
        0
    },
    {},
};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);
static struct  i2c_driver ssd1306_driver = 
{
    .probe = ssd_probe,
    .remove = ssd_remove,
    .driver = 
    {
        .name = "my_driver_ssd1306",
        .of_match_table = ssd1306_driver_id,
    },
    .id_table = ssd1306_id,
};

//fops
static int my_open(struct inode* inode, struct file* file);
static int my_release(struct inode* inode, struct file* file);
static ssize_t my_read(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t my_write(struct file* file, const char __user* buf, size_t len, loff_t* off);
// static ssize_t my_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf);
// static ssize_t my_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count); 
// static struct kobj_attribute ssd_attr = __ATTR(val_sys, 0660, my_show, my_store);
struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write, 
    .release = my_release,
};

static int my_open(struct inode* inode, struct file* file)
{
    pr_info("OPEN\n");
    struct ssd1306_t* ssd;
    ssd = container_of(inode->i_cdev, struct ssd1306_t, my_cdev);
    file->private_data = ssd;
    return 0;
}
static int my_release(struct inode* inode, struct file* file)
{
    pr_info("RELEASE\n");
    return 0;
}
static ssize_t my_read(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    pr_info("READ\n");
    pr_info("TEST SSD\n");
    struct ssd1306_t* ssd = file->private_data;
    count++;
    if(count > 9) count = 0;
    pr_info("Count = %d\n", count);
    // ssd1306_send_cmd(ssd, SSD1306_ENTIRE_DISPLAY_ON);
    ssd1306_write_integer_8x8(ssd, count);
    // ssd1306_draw_logo(ssd);
    return 0;
}
static ssize_t my_write(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    char k_buf[32];
    int page, col;
    int is_data = 0;
    struct ssd1306_t* ssd = file->private_data;
    if(copy_from_user(k_buf, buf, len))
    {
        pr_err("Cannot write\n");
        return -1;
    }
    k_buf[len] = '\0';
    if(k_buf[0] == 'C')
    {
        if(k_buf[1] = 'L')
        {
            ssd1306_clear(ssd);
        }
        sscanf(k_buf, "C %d %d", &col, &page);
        pr_info("Col = %d\t Page = %d \n", col, page);
        ssd1306_set_page_col(ssd, col, page);
    }
    else if(k_buf[0] == 'D')
    {
        char str[32];
        sscanf(k_buf, "D %s", str);
        pr_info("k_buf: %s\n", str);
        ssd1306_write_string(ssd, str);
        is_data = 1;
    }
    else;
    if(is_data)
    {
        ssd1306_write_space(ssd);
        is_data = 0;
    }
    return len;
}

static int ssd_probe(struct i2c_client* client, const struct i2c_device_id* id)
{   
    struct device* dev = &(client->dev);
    uint32_t height, width;
    struct ssd1306_t* ssd;
    ssd = devm_kmalloc(dev, sizeof(struct ssd1306_t), GFP_KERNEL);
    if(ssd == NULL)
    {
        pr_err("Cannot allocate memory for ssd\n");
        return -1;
    }
    ssd->client = client;
    i2c_set_clientdata(client, ssd);
    int ret;
    if(device_property_present(dev, "height") == false)
    {
        dev_err(dev, "'height' not found\n");
        return -1;
    }
    if(device_property_present(dev, "width") == false)
    {
        dev_err(dev,"'width' not found\n");
        return -1;
    }
    ret = device_property_read_u32(dev, "height", &height);
    if(ret)
    {
        dev_err(dev, "Cannot read height\n");
        return -1;
    }
    ret = device_property_read_u32(dev, "width", &width);
    if(ret)
    {
        dev_err(dev, "Cannot read width\n");
        return -1;
    }
    dev_info(dev, "Height = %d\tWidth = %d\n", height, width);
    if(alloc_chrdev_region(&ssd->dev_num, 0, 1, "ssd_num") < 0)
    {
        pr_err("Cannot create major minor number\n");
        return -1;
    }
    pr_info("Major(%d) \t Minor(%d)\n", MAJOR(ssd->dev_num), MINOR(ssd->dev_num));
    ssd->dev_class = class_create(THIS_MODULE, "ssd_class");
    if(IS_ERR(ssd->dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    if(IS_ERR(device_create(ssd->dev_class, NULL, ssd->dev_num, NULL, "ssd_device")))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    cdev_init(&ssd->my_cdev, &my_fops);
    if(cdev_add(&ssd->my_cdev, ssd->dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    ssd1306_init(ssd);
    ssd1306_set_page_col(ssd, 0, 0);
    // ssd1306_draw_bitmap(ssd, 0, 0, bitmap_sawtooth, 128, 8);
    // // ssd1306_draw_bitmap(ssd, 10, 2, bitmap_smile_icon, 16, 16);
    // ssd1306_draw_bitmap(ssd, 0, 2, bitmap_turtle, 32, 32);
    // ssd1306_draw_bitmap(ssd, 33, 2, bitmap_cat, 32, 32);
    // ssd1306_draw_bitmap(ssd, 66, 2, bitmap_cow, 32, 32);
    // ssd1306_draw_bitmap(ssd, 99, 2, bitmap_hotdog, 32, 32);
    // ssd1306_set_page_col(ssd, 10, 7);
    ssd1306_write_integer_8x8(ssd, 0);
    pr_info("Insert done\n");
    return 0;
r_device:
    class_destroy(ssd->dev_class);
r_class:
    cdev_del(&ssd->my_cdev);
    unregister_chrdev_region(ssd->dev_num, 1);
    return -1;
}
static void ssd_remove(struct i2c_client* client)
{
    struct ssd1306_t* ssd = i2c_get_clientdata(client);
    // ssd1306_send_cmd(ssd, SSD1306_ENTIRE_DISPLAY_OFF);
    ssd1306_clear(ssd);
    ssd1306_send_cmd(ssd, SSD1306_DISPLAY_OFF);
    device_destroy(ssd->dev_class, ssd->dev_num);
    class_destroy(ssd->dev_class);
    cdev_del(&ssd->my_cdev);
    unregister_chrdev_region(ssd->dev_num, 1);
    pr_info("remove i2c ssd1306 done\n");
}

module_i2c_driver(ssd1306_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");