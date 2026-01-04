#include <linux/module.h>
#include "linux/button.h"
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/property.h>
#include <linux/device.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/interrupts.h>
int button_probe(struct platform_device *pdev);
int button_remove(struct platform_device *pdev);
static struct of_device_id button_id[] = 
{
    {
        .compatible = "solomon,button_ssd1306_ui",
    },
    {},
};

MODULE_DEVICE_TABLE(button_id);

static struct platform_driver button_ssd = 
{
    .probe = button_probe,
    .remove = button_remove,
    .driver = 
    {
        .of_match_table = button_id,
        .name = "button_driver",
    },
};
struct button_t 
{
    gpio_desc *btn[NUMBER_BUTTON];
    int irq[NUMBER_BUTTON];
};
int button_probe(struct platform_device *pdev)
{
    struct device* dev = &pdev->dev;
    char* label;
    uint32_t debounce; 
    struct button_t* data;
    btn = devm_kzalloc(dev, sizeof(button_t), GFP_KERNEL);
    if(device_property_read_string(dev, "label", &label))
    {
        dev_err(dev, "Cannot read string\n");
        return -1;
    }
    if(device_property_read_u32(dev, "debounce-ms", &debounce))
    {
        dev_err(dev, "Cannot read debounce\n");
        return -1;
    }
    dev_info(dev, "Label: %s\n", label);
    dev_info(dev, "Debounce_ms: %d\n", debounce);
    for(int i = 0; i < NUMBER_BUTTON; i++)
    {
        data->btn[i] = devm_gpiod_get_index(dev, "button", i, GPIOD_IN);
        if(IS_ERR(data->btn[i]))
        {
            dev_err(dev, "Cannot get id of btn\n");
            return -1;
        }
        gpiod_set_debounce(data->btn[i], debounce);
        data->irq[i] = gpiod_to_irq(data->btn[i]);
        if(devm_request_irq())
    }
}