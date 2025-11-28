#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>


static int my_usb_probe(struct usb_interface* intf, const struct usb_device_id* id)
{
    dev_info(intf->dev, "USB DRIVER PROBE:\n Vendor id: 0x%02x, \t, Product id: 0x%02x\n", id->idVendor, id->idProduct);
    return 0;
}
