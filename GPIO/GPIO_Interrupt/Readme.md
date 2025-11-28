# GPIO_INTERRUPTS

## Functions

1. GPIO_SET_DEBOUNCE
This function uses to set debounce time for a gpio.  
```c
gpio_set_debounce(unsigned gpio, unsigned debounce);
```
It's return 0 if successful.  

2. Get the irq number for gpio
This function is used to get irqnumber for gpio.  
```c
int gpio_to_irq(unsigned gpio);
```

3. Request the irq (flags)
Interrupt Flags  
```c
IRQF_TRIGGER_RISING  
IRQF_TRIGGER_FALLING
IRQF_TRIGGER_HIGH   
IRQF_TRIGGER_LOW
```


