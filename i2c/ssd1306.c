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
//define cmd ssd1306
#define SINGLE_CMD 0x00    //dc = 1, co = 0
#define SINGLE_DATA 0x40   //dc = 1, co = 1
enum ssd1306_cmd 
{
    SSD1306_DISPLAY_ON = 0xAF,  //normal mode
    SSD1306_DISPLAY_OFF = 0xAE, //sleep mode
    SSD1306_SET_CLOCK_DIV_RATIO = 0XD5, //default = 0x80
    SSD1306_SET_MULTIPLEX_RATIO = 0xA8, //default 128x64 = 63, 128x32 = 31
    SSD1306_SET_DISPLAY_OFFSET = 0xD3,  //00 - 63
    SSD1306_SET_DISPLAY_START_LINE = 0x40,  //0x40 - 0x7F (00-63)
    SSD1306_SET_CHARGE_PUMP = 0x8D, //and send 0x14 to enable, 0x10: disable
    SSD1306_MEMORY_MODE = 0x20, //0x00: Horizontal, 0x01: Vertical
    //left <-> right
    SSD1306_REMAP_NORMAL = 0xA0,    
    SSD1306_REMAP_REVERSE = 0xA1,
    // top <-> bottom
    SSD1306_SCAN_DIRECTION_NORMAL = 0xC0,
    SSD1306_SCAN_DIRECTION_REVERSE = 0xC8,
    SSD1306_SET_COM_PIN = 0xDA, //64: 0x12, 32: 0x02
    SSD1306_SET_CONTRAST = 0x81,
    SSD1306_SET_PRE_CHARGE = 0xD9, //normally 0xF1 (1111: phase 2 pre-charge, 0001: phase 1 Discharge)
    SSD1306_SET_VCOMH_DESELECT = 0xDB,   //A[6:4] (USE 0X20)
    SSD1306_ENTIRE_DISPLAY_ON  = 0xA5, // Entire display on (don't care Ram)
    SSD1306_ENTIRE_DISPLAY_OFF = 0xA4, //Entire display off
    SSD1306_NORMAL_DISPLAY = 0xA6,  //Ram = 1 -> pixel on
    SSD1306_INVERSE_DISPLAY = 0xA7, //ram = 0 -> pixel off
};

static int ssd_probe(struct i2c_client* client, const struct i2c_device_id* id);
static void ssd_remove(struct i2c_client* client);

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

struct ssd1306_t
{
    struct i2c_client* client;
    dev_t dev_num;
    struct class* dev_class;
    // struct device* dev_file,
    struct cdev my_cdev;
    struct kobject* my_kobj;
    // struct 
};

// //function write
static int i2c_write(struct ssd1306_t* ssd ,unsigned char* buf, unsigned int len);
static void ssd1306_send_cmd(struct ssd1306_t* ssd, enum ssd1306_cmd cmd);

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
    ssd1306_send_cmd(ssd, SSD1306_ENTIRE_DISPLAY_ON);
    return 0;
}
static ssize_t my_write(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}

static int i2c_write(struct ssd1306_t* ssd ,unsigned char* buf, unsigned int len)
{
    int ret = i2c_master_send(ssd->client, buf, len);
    return ret;
}

static void ssd1306_send_cmd(struct ssd1306_t* ssd, enum ssd1306_cmd cmd)
{
    unsigned char buf[2] = {SINGLE_CMD, cmd};
    int ret = i2c_write(ssd, buf, 2);
}
static void ssd1306_send_data(struct ssd1306_t* ssd, unsigned char* data)
{
    unsigned char buf[2] = {SINGLE_DATA, data};
    int ret = i2c_write(ssd, buf, 2);
}
static void ssd1306_init(struct ssd1306_t* ssd)
{
    msleep(100);
    ssd1306_send_cmd(ssd, SSD1306_DISPLAY_OFF);
    //set retio
    ssd1306_send_cmd(ssd, SSD1306_SET_MULTIPLEX_RATIO);
    ssd1306_send_cmd(ssd, 0x3F);
    //set display off set
    ssd1306_send_cmd(ssd, SSD1306_SET_DISPLAY_OFFSET);
    ssd1306_send_cmd(ssd, 0x00);
    //Set display start line
    ssd1306_send_cmd(ssd, SSD1306_SET_DISPLAY_START_LINE);
    //memory mode 
    ssd1306_send_cmd(ssd, SSD1306_MEMORY_MODE);
    ssd1306_send_cmd(ssd, 0x00);
    // Remap disable
    ssd1306_send_cmd(ssd, SSD1306_REMAP_NORMAL);
    //scan com
    ssd1306_send_cmd(ssd, SSD1306_SCAN_DIRECTION_NORMAL);
    //set compin hw
    ssd1306_send_cmd(ssd, SSD1306_SET_COM_PIN);
    ssd1306_send_cmd(ssd, 0x12);
    //set contrast
    ssd1306_send_cmd(ssd, SSD1306_SET_CONTRAST);
    ssd1306_send_cmd(ssd, 0x7F);
    // entire display off
    ssd1306_send_cmd(ssd, SSD1306_ENTIRE_DISPLAY_OFF);
    //set normal display
    ssd1306_send_cmd(ssd, SSD1306_NORMAL_DISPLAY);
    // set osc frequency
    ssd1306_send_cmd(ssd, SSD1306_SET_CLOCK_DIV_RATIO);
    ssd1306_send_cmd(ssd, 0x80);
    //charge pump
    ssd1306_send_cmd(ssd, SSD1306_SET_CHARGE_PUMP);
    ssd1306_send_cmd(ssd, 0x14);    //enable
    ssd1306_send_cmd(ssd, SSD1306_DISPLAY_ON);
    // ssd1306_send_cmd(ssd, SSD1306_ENTIRE_DISPLAY_ON);
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
    ssd1306_send_cmd(ssd, SSD1306_ENTIRE_DISPLAY_OFF);
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