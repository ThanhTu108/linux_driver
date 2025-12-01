#include <linux/module.h>
#include <linux/init.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/err.h>

static int dt_probe(struct platform_device* pdev);
static int dt_remove(struct platform_device* pdev);
 
static struct of_device_id my_drive_id[] = 
{
    {
        .compatible = "test_dt,test_dev",
    },
    {}
};

static struct platform_driver my_driver = 
{
    .probe = dt_probe,
    .remove = dt_remove,
    .driver = 
    {
        .name = "my_device_driver",
        .of_match_table = my_drive_id,
    },
};

static int dt_probe(struct platform_device* pdev)
{
    struct device* dev = &pdev->dev;
    const char* label;
    int my_value, ret;
    pr_info("probe function \n");
    if(device_property_present(dev, "label") == false)
    {
        pr_err("dt_probe - err!!, 'label' not found\n");
        return -1;
    }
    if(device_property_present(dev, "my_val") == false)
    {
        pr_err("dt_probe - err!!, 'my_val' not found\n");
        return -1;
    }
    
    //read device properties 
    ret = device_property_read_string(dev, "label", &label);
    if(ret)
    {
        pr_err("Cannot read string 'label'\n");
        return -1;
    }
    ret = device_property_read_u32(dev, "my_val", &my_value);
    if(ret)
    {
        pr_err("Cannot read 'my_val'\n");
        return -1;
    }
    pr_info("Lable = %s\n", label);
    pr_info("My_value = %d\n", my_value);
    return 0;
}

static int dt_remove(struct platform_device* pdev)
{
    pr_info("Remove probe\n");
    return 0;
}

static int __init cre_probe(void)
{
    pr_info("Load probe function\n");
    if(platform_driver_register(&my_driver))
    {
        pr_info("Cannot load driver\n");
        return -1;
    }
    pr_info("Load driver done\n");
    return 0;
}
static void __exit rmv_probe(void)
{
    pr_info("Remove prove\n");
    platform_driver_unregister(&my_driver);
}
module_init(cre_probe);
module_exit(rmv_probe);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");