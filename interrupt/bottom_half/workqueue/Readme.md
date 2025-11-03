# Workqueue 
Workqueue are add in the linux kernel version 2.6. <br>
Workqueue are used to schedule work to be done later in process context. Workqueue run in process context and can sleep. <br>

If the deferred work needs to sleep, then workqueue is used. <br>
If the deferred work needs not sleep, then softirq or tasklet are used. <br>

In this example we are using Workqueue to implement Bottom Half. 
## Implementation workqueue
1. Using global workqueue (static / dynamic)
2. Create **own workqueue**

### Static Global Workqueue
**Initialize work structure** <br>
```c
DECLARE_WORK(name, void (*func)(void *));
```
- where 
    - name: name of the work structure <br>
    - func: the function to be scheduled in the workqueue <br>
Ex: 
```c
static DECLARE_WORK(workqueue, workqueue_fn);
```
**Schedule work to the workqueue**
1. Schedule_work:
This function puts a job in the kernel-global workqueue.
```c
int schedule_work(struct work_struct* work);
```
where:
- work: job to be done <br>
Return 0 if work was already on a queue, non-zero otherwise. <br>
2. Schedule_delayed_work:
After waiting for a given time this function puts a job in the kernel-global workqueue.
```c
int schedule_delayed_work(struct delayed_work* dwork, unsigned long delay);
```
where:
- dwork: job to be done <br>
- delay: number of jiffies to wait or 0 for immediate execution <br>
Return 0 if work was already on a queue, non-zero otherwise. <br>
3. Schedule_work_on:
Puts a job in the workqueue of a specific CPU.
```c
int schedule_work_on(int cpu, struct work_struct* work);
```
where:
- cpu: CPU to puts the work on <br>
- work: job to be done <br>
Return 0 if work was already on a queue, non-zero otherwise. <br>
4. Schedule_delayed_work_on:
After waiting for a given time this function puts a job in the workqueue of a specific CPU.
```c
int schedule_delayed_work_on(int cpu, struct delayed_work* dwork, unsigned long delay);
```
where: 
- cpu: CPU to puts the work on <br>
- dwork: job to be done <br>
- delay: number of jiffies to wait or 0 for immediate execution <br>
Return 0 if work was already on a queue, non-zero otherwise. <br>
**Delete work from the workqueue**
int flush_work(struct work_struct* work);
void flush_scheduled_work(void);

