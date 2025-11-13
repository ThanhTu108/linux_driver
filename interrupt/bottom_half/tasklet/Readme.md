# Tasklets

Tasklets are used to handle bottom half processing in linux kernel interrupts. Tasklets can be run in parallel but the same tasklet cannot be run on multiple CPUs at the same time.
Each tasklet run only on the CPU on which it was scheduled.

**Remember**
- Tasklets run in interrupt context, so they cannot sleep.
- Tasklets have a higher priority than softirqs.
- Tasklet can be run on the same CPU that scheduled it
- Tasklet are executed by the principal of non-preemptive scheduling, one by one. We can schedule them with two different priorities: high and normal.

## Static method
1. tasklet_struct
```c
struct tasklet_struct
{
    struct tasklet_struct* next;
    unsigned long state;
    atomic_t count;
    void (*func)(unsigned long);
    unsigned long data;
};
```
- where:
    - next: the next tasklet in line for scheduling
    - state: the state of the tasklet, TASKLET_STATE_SCHED (Scheduled) or TASKLET_STATE_RUN (Running)
    - count: 0 if enabled, >0 if disabled
    - func: pointer to the function to be executed by the tasklet
    - data: data to be passed to the function


2. DECLARE_TASKLET(name, func, data)
```c
#define DECLARE_TASKLET(name, func, data);
```
- where:
    - name: name of the tasklet
    - func: function to be executed by the tasklet
    - data: data to be passed to the function
Ex:
```c
DECLARE_TASKLET(tasklet,tasklet_fn, 1);
```
This function will create a tasklet named 'tasklet' that will execute the function 'tasklet_fn' with the data '1'.
```c
struct tasklet_struct tasklet = { NULL, 0, 0, my_tasklet_function, 1};
struct tasklet_struct my_tasklet =
{
    .next = NULL,
    .state = TASKLET_STATE_SCHED,
    .count = 0,     //enabled
    .func = my_tasklet_function,
    .data = 1,     //data to be passed to the function
}
```
3. DECLARE_TASKLET_DISABLED(name, func, data)
This function is similar to DECLARE_TASKLET but the tasklet is created in a disabled state. Which means it can be scheduled but will not run until it is enabled.
```c
DECLARE_TASKLET_DISABLED(name, func, data);
```
- Where:
    - name: name of the tasklet
    - func: function to be executed by the tasklet
    - data: data to be passed to the function

Ex:
```c
DECLARE_TASKLET_DISABLED(tasklet,tasklet_fn, 1);
```
4. Enable and Disable tasklet
- Enable tasklet
```c
void taslet_enable(struct tasklet_struct* t);
```
- Disable tasklet: this function will disable the tasklet and wait for it to finish if it is currently running.
```c
void tasklet_disable(struct tasklet_struct* t);
```
- Disable tasklet with no wait: this function will disable the tasklet but will not wait for it to finish if it is currently running.
```c
void tasklet_disable_nosync(struct tasklet_struct* t);
```
5. Schedule tasklet
```c
void tasklet_schedule(struct tasklet_struct* t);
```
This function will schedule the tasklet to be run at the next available opportunity.
6. Schedule high priority tasklet
```c
void tasklet_hi_schedule(struct tasklet_struct* t);
```
This function will schedule the tasklet to be run at the next available opportunity with high priority.
7. Kill tasklet
```c
void tasklet_kill(struct tasklet_struct* t);
```
This function will disable the tasklet and wait for it to finish if it is currently running.

## Dynamic method
1. Tasklet init
```c
void tasklet_init(struct tasklet_struct* t, void(*)(unsigned long) func,unsigned long data);
```
This function will initialize the tasklet with the given function and data.
- where:
    - t: pointer to the tasklet to be initialized
    - func: function to be executed by the tasklet
    - data: data to be passed to the function
Ex:
```c
struct tasklet_struct my_tasklet;
tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
if(!tasklet)
{
    pr_info("Cannot allocate memory for tasklet\n");
    return -1;
}
tasklet_init(tasklet, my_tasklet_function, 1);
```
when i call function tasklet_init, it is similar to:
```c
tasklet->func = my_tasklet_function;
tasklet->data = 1;
tasklet->state = TASKLET_STATE_SCHED;
atomic_set(&tasklet->count, 0); //enabled
