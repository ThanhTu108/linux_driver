# High Resolution Timer
Kernel Timers are bound to jiffies. But this High Resolution Timer (HRT) is bound with 64-bit nanoseconds resolution.  

In the /boot directory check the kernel config **CONFIG_HIGH_RES_TIMERS=y**  

HR timer is define at <linux/hrtimer.h>  
```c
struct hrtimer {
	struct timerqueue_node		node;
	ktime_t				_softexpires;
	enum hrtimer_restart		(*function)(struct hrtimer *);
	struct hrtimer_clock_base	*base;
	u8				state;
	u8				is_rel;
	u8				is_soft;
	u8				is_hard;
};

struct timerqueue_node {
	struct rb_node node;
	ktime_t expires;
};
```
- where:
	- hrtimer_restart: 
		+ HRTIMER_RESTART: periodic.
		+ HRTIMER_NORESTART: one-shot.
	- base: pointer to clock base (CLOCK_MONOTONIC, CLOCK_REALTIME)
	- state: status of timer
		+ HRTIMER_STATE_INACTIVE: not run yet
		+ HRTIMER_STATE_ENQUEUED: in RB-Tree
		+ HRTIMER_STATE_CALLBACK: run in softirq
	- is_rel: relative or absolute
		+ 1: relative
		+ 0: absolute
	- is_soft: 
		+ 1: callback run in softirq
		+ 0: hard irq context
	- is_hard: run in hard irq
## Function

### 1. ktime_set()
This function is used to store a time value in nanoseconds (ktime_t)
```c
ktime_t ktime_set(const s64 secs, const unsigned long nsecs);
```
- where: 
	- secs: seconds to set
	- nsecs: nanosecond to set
The ktime_t is representation of the value  
### 2. Init 
This function is used to initialize timer want to use.
```c
void hrtimer_init(struct hrtimer* timer, clockid_t clock_id, enum hrtimer_mode mode);
```
- where:
	- timer: timer to be initialize
	- clock_id: the clock to be used
		+ CLOCK_MONOTONIC: a clock always to move forward in time (not real time world)  
		+ CLOCK_REALTIME: match with current real world time  
	- mode: timer mode  
		+ HRTIMER_MODE_ABS: absolute mode  
		+ HRTIMER_MODE_REL: relative mode  
### 3. Start/Stop high resolution timer
**Start high resolution timer**  
This function is used to start or restart a hrtimer on CPU.
```c
void hrtimer_start(struct hrtimer* timer, ktime_t tim, const enum hrtimer_mode mode);
```
- where: 
	- timer: the timer to be add
	- tim: expiry time  
	- mode: absolute/ relative mode  
**Stop high resolution timer** 
This function is used to stop high resolution timer  
```c
void hrtimer_cancel(struct hrtimer* timer);
```
- where: timer: the timer to be canceled  

This function try to deactive timer  
```c
int hrtimer_try_to_cancel(struct hrtimer* timer);
```
It's return: 
 - 0: the timer was not active  
 - 1: the timer was active  
 - -1: the timer is currently execute callback function  

### 4. Hrtimer forward  
This function is used to forward the timer expiry so it will expiry in the time by the given interval  
```c
u64 hrtimer_forward(struct hrtimer* timer, ktime_t now, ktime_t interval);
```
- where: 
	- timer: timer to be forward  
	- now: forward this time
	- interval: the interval to forward  

Returns the number of overruns.  

### 5. Check hr timer's status
**hrtimer_get_remaining**  
This function is used to get remaining time for the timer  
```c
ktime_t hrtimer_get_remaining(const struct hrtimer * timer);
```
- where:
	- timer: the timer to be check  
Return the remainning time  

**hrtimer_callback_running**  
This function is used to check the timer is running the callback function  
```c
int hrtimer_callback_running(struct hrtimer* timer);
```
Return 0: callback function is not runninng, 1 otherwise  

