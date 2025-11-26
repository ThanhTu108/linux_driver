# USB_DEVICE
The usb system consits of USB core, USB host controller, USB host controller's driver
![alt text](System_usb.png)

1. USB device
2. USB host controller
This is used to communications between host system and USB device. USB host controller contain both of hardware and software:
- Hardware:
    - Detect connet/disconnect USB device.
    - Providing power to connect USB device
- Software:
    - Call USB host controller driver, load USB device driver, manage data transfer.
    - Assign the appropriate USB device driver to the device.  
        + Open host controller interface (OHCI) USB 1.X
        + Universal host controller interface (UHCI) USB 1.X
        + Enhanced host controller interface (EHCI) USB 2.X
        + Extend host controller interface (XHCI) USB 3.X

3. USB core
Usb core is a  codebase consisting of rountine and struct available to host controller driver and usb driver.  
4. USB driver
Driver which write for USB device.  
5. USB descriptor
The USB device contains of a number of descriptor that help to define what the device is capable of. 

- Device descriptor:
    + USB device only have 1 device descriptor. Device descriptor include information revision USB, product, vendor id  

- Configuration descriptor:  
    + The configuration descriptor specifies values such as the amount of power this particular configuration uses if the device is self or bus-powered and the number of interfaces it has. 
    + When a device is enumerated, the host reads the device descriptors and can make a decision of which configuration to enable. 
    + A device can have more than one configuration, though it can enable only one configuration at a time.
- Interface descriptor: 
    + A device can have one or more interfaces. Each interface can have a number of endpoint and represents a function 
- Endpoint descriptor:
    + Each endpoint descriptor is used to specify the type of transfer, direction, polling interval, and maximum packet size for each endpoint


![alt text](Flow_of_usb_device.png)

## Data flow types
There are 4 difference ways to transfer data on a USB bus. Each has its own purposes. Each one is built up using one or more transaction types. Set it into endpoint descriptor.  
- Control transfers  
- Interrupt transfers  
- Bulk transfers
- Isochronous transfers



# USB_DRIVER_API
## 1. Usb_driver struct
Before register usb device, we need to give some information about device. All information is pass to the usb subsystem via usb_driver struct
```c
struct usb_driver {
	const char *name;

	int (*probe) (struct usb_interface *intf,
		      const struct usb_device_id *id);
	void (*disconnect) (struct usb_interface *intf);
	int (*unlocked_ioctl) (struct usb_interface *intf, unsigned int code,
			void *buf);
	int (*suspend) (struct usb_interface *intf, pm_message_t message);
	int (*resume) (struct usb_interface *intf);
	int (*reset_resume)(struct usb_interface *intf);
	int (*pre_reset)(struct usb_interface *intf);
	int (*post_reset)(struct usb_interface *intf);
	const struct usb_device_id *id_table;
	const struct attribute_group **dev_groups;
	struct usb_dynids dynids;
	struct usbdrv_wrap drvwrap;
	unsigned int no_dynamic_id:1;
	unsigned int supports_autosuspend:1;
	unsigned int disable_hub_initiated_lpm:1;
	unsigned int soft_unbind:1;
};
```
- where: 
    + name: the driver name 
    + probe: The function need to call when a USB device is connected
    + disconnect: The function need to call when a USB device is disconnected
    + Ioctl: Used for drivers that want to talk to userspace through the “usbfs” filesystem.
    + suspend: Call when the device is going to be subsended by the system
    + .....:
    + id_table: usb driver use an id_table to support hotplugging 
    +  

## 2. Id_table:
The id_table used in hotplugging
```c
const struct usb_device_id* my_id_table = 
{
    {USB_DEVICE(vendor_id, product_id)},
    {},
};
```

## 3. Probe
Probe function is called by the kernel when a device match the id_table (Hot pluggin)
```c
// int (*probe) (struct usb_interface *intf,
// 		      const struct usb_device_id *id);

static int my_probe(struct usb_interface* intf, const struct usb_device_id* id)
{
    dev_info(&intf->dev, "USB Driver Probed: Vendor ID : 0x%02x,\t"
             "Product ID : 0x%02x\n", id->idVendor, id->idProduct);
    
    return 0;
}
```
## 4. Disconnect

```c
static void my_disconnect(struct usb_interface* intf)
{
    dev_info(&intf->dev, "USB Driver Disconnected\n");
}
```

```c
static struct usb_driver my_usb_driver = 
{
    .name = "my_usb_driver",
    .probe = my_probe,
    .disconnect = my_disconnect,
    .id_table = my_id_table,
};
```

## Regiter the usb driver to the usb subsystem 
```c
usb_register(struct usb_driver* ud);
```
Ex:
```c
usb_register(&my_usb_driver);
```

## De-regiter the usb driver to the usb subsystem 
```c
usb_deregister(struct usb_driver* ud);
```
Ex:
```c
usb_deregister(&my_usb_driver);
```

## init and exit
```c
#define module_usb_driver(__usb_driver) \
	module_driver(__usb_driver, usb_register, \
		       usb_deregister)
```
Ex:
```c
module_usb_driver(my_usb_driver);
```

