# Workqueue 
Workqueue are add in the linux kernel version 2.6. <br>
Workqueue are used to schedule work to be done later in process context. Workqueue run in process context and can sleep. <br>

If the deferred work needs to sleep, then workqueue is used. <br>
If the deferred work needs not sleep, then softirq or tasklet are used. <br>

In this example we are using Workqueue to implement Bottom Half. 
## Implementation workqueue
1. Using global workqueue (static / dynamic)
2. Create **own workqueue**
