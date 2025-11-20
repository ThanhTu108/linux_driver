# Timer in linux kernel
In linux, every time a timer interrupts occurs, the value of an internal counter incremented. The counter is initialize to 0 at the system boot.
**Usage**
- Checking the status of the device at regular intervals when the hardware can't fire interrupts
- The user want to send some infomation/ message to another device at regular intervals
## Function
You need to import library 
```c
#include <linux/timer.h>
```
The timer struct is define as: 
```c
struct timer_list
{
    struct hlist_node entry;
    unsigned long expires;
    void (*function)(struct timer_list *);
	u32	flags;
};
```
Where:
    - Expires: expiration time of the timer (jiffies/ tick)
    - When expires is occur the function will be call with the data is given

### Initialize timer / setup
1. Init timer
This function is used to initialize the timer.
**In the older version**
```c
void fastcall init_timer();
```
Or: 
```c
void setup_timer(timer, function, data);
```
And which my kernel version: 6.1.83 i will use this api: timer_setup

**New vesion**
```c
void timer_setup(timer, function, data);
```
- Where:
    - timer: the timer to be initialize
    - function: Callback function to be call when the timer expires. The function pointer is: void (*function)(struct timer_list *);
    - data: data is given to callback function
Ex:
```c
struct timer_list timer;
timer_setup(&timer, timer_callback, 0);
void timer_callback(struct timer_list* t)
{
    //Do something;
}
```

**Define**
If you using this method, you cannot create struct timer_list. The kernel will create the struct in the name of name in initialize
```c
DEFINE_TIMER(name, function, expires, data);
```
- Where:
    - name: the name of struct timer_list;
    - function: Callback function to be call when the timer expires. The function pointer is: void (*function)(struct timer_list *);
    - expires: expiration time of the timer (jiffies/ tick);
    - data: data is given to callback function;

### Start a kernel timer
This function will start a timer
```c
void add_timer(struct timer_list* timer);
```
timer: The timer need to start

### Modify linux kernel timer (expires)
This function is used to modify timer's timeout. Normally this use to update expire timer if the timer is inactive it will be active
```c
int mod_timer(struct timer_list* timer, unsigned long expires);
```
When you use this funtion it like:
```c
del_timer(timer);
timer->expires = expires;
add_timer(timer);
```

This function return  
0 – mod_timer of an inactive timer
1 – mod_timer of an active timer

### Stop a kernel timer
This will deactive the timer. Works onboth active and inactive timers.
```c
int del_timer(struct timer_list* timer);
```
This return:
    - 0: inactive timer
    - 1: active timer
```c
int del_timer_sync(struct timer_list* timer);
```
This function will deactive timer and wait for the handler to finish.

### Check the kernel timer
This function is used to check if timer is currently pending or not
```c
int timer_pending(const struct timer_list * timer);
```
This return:
    - 0: timer is not pending  
    – 1: timer is pending  