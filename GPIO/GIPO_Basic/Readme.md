# GPIO_BASIC

There are 7 ways to access the gpio from the kernel GPIO Subsystem.
1. Check the gpio is valid or not
2. Request the gpio from the kernel GPIO Subsystem.
3. Export the GPIO to sysfs (optional)
4. Set the direction of the GPIO
5. Make the gpio 1/0 if gpio is set an output
6. Set the debounce interval and read state if gpio is set an input.
7. Release GPIO  

## Function 

You need to include GPIO header file 
```c
#include <linux/gipo.h>
```

1. Validate the gpio
Before using gpio, You must check whether the gpio is valid or not.
```c
bool gpio_is_valid(int gpio_number);
```
- where:
    + gpio_number: GPIO that you plant to use.
It's return true if valid else false.
2. Request the gpio
Once you have a valid gpio, you can request the gpio
```c
int gpio_request(unsigned gpio, const char* label);
```
- where:
    + gpio: GPIO that you plant to use.
    + label: any string. That can be seen in "/sys/kernerl/debug/gpio".
It's return 0 if success and negative number if fail
```c
int gpio_request_one(unsigned gpio, unsigned long flags, const char *label); // Request one GPIO.
int gpio_request_array(struct gpio *array, size_t num); //Request multiple GPIOs. 
```
3. Export gpio (optional)
For debug you can export the gpio which is allocated using the gpio_request() to the sysfs using this API  
```c
int gpio_export(unsigned gpio, bool direction_may_change);
```
- where:
    + gpio: GPIO that you plant to use.
    + direction_may_change: true (Can change) or false.
4. Unexport gpio 
```c
void gpio_unexport(unsigned int gpio);
```
5. Set the direction
Set gpio as output/input  
**Set gpio input**  
```c
int gpio_direction_input(unsigned gpio);
```
- where:
    + gpio: GPIO that you plant to use.
It's return 0 if success.  

**Set gpio output**
```c
int  gpio_direction_output(unsigned gpio, int value);
```
- where:
    + gpio: GPIO that you plant to use.
    + value: the value of gpio (begin status).
It's return 0 if success.  

6. Change gpio value
```c
gpio_set_value(unsigned int gpio, int value);
```
7. Read gpio value
```c
int  gpio_get_value(unsigned gpio);
```
8. Release gpio
```c
void gpio_free(unsigned int gpio);
```