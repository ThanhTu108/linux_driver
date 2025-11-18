# Spinlock
Spin lock is used to protected a shared resource

## Functions
### Init 
1. Static method
Using macro below to initialize Spinlock
```c
DEFINE_SPINLOCK(name);
```
This macro will create struct spinlock_t with name and initialize to **UNLOCKED_STATE**
```c
#define DEFINE_SPINLOCK(x)      spinlock_t x = __SPIN_LOCK_UNLOCKED(x)
```
2. Dynamic method
```c
spinlock_t name;
spin_lock_init(&name);
```

After initialize the spinlock, there are several way to lock and unlock base on where the spinlock is used
## Implementation
### 1. Locking between user context / Locking between bottom half
If you share data with user context (between kernel thread), you can use this function:
**Lock**
```c
spin_lock(spinlock_t* lock);
```
This will take the lock if it free, otherwise it will spin until lock is free
**Try lock**
```c
spin_trylock(spinlock_t* lock);
```
This will take the lock if it free. Otherwise it exis with an error and does not spin. Return non-zero if it take the lock otherwise returns 0  
**Un lock**
```c
spin_unlock(spinlock_t* lock);
```
**Checking lock**
```c
spin_is_locked(spinlock_t* lock);
```
This is used to check whether the lock is available or not. It returns non-zero if the lock is currently acquired. otherwise returns zero.
### 2. Locking between user context and bottom half
If you share data with a bottom half and user context (like kernel thread), this approach is useful
**Lock**
```c
spin_lock_bh(spintlock_t* lock);
```
This will disable softirq on CPU and take the lock if it free. this has the effect of preventing bottom half from running on the local CPU.
**Unlock**
```c
spin_unlock_bh(spinlock_t *lock);
```
Ex:
```c
int thread_function(void *pv)
{
    while(!kthread_should_stop()) {
        spin_lock_bh(&etx_spinlock);
        val++;
        pr_info("Thread Value = %lu\n", val);
        spin_unlock_bh(&etx_spinlock);
        msleep(1000);
    }
    return 0;
}
/*Tasklet Function*/
void tasklet_fn(unsigned long arg)
{
        spin_lock_bh(&etx_spinlock);
        val++;
        pr_info("Tasklet Value = %lu\n", val);
        spin_unlock_bh(&etx_spinlock);
}
```

### 3. Locking between hard irq and bottom half
Using:
```c
spin_lock_irq(spinlock_t *lock);\
spin_unlock_irq(spinlock_t *lock);
```
Ex:
```c
void tasklet_fn(unsigned long arg)
{
        spin_lock_irq(&etx_spinlock);
        val++;
        pr_info("Tasklet Value = %lu\n", val);
        spin_unlock_irq(&etx_spinlock);
}
static irqreturn_t irq_handler(int IRQ, void* dev_id)
{
        spin_lock_irq(&etx_spinlock);
        val++;
        pr_info("Itr Value = %lu\n", val);
        spin_unlock_irq(&etx_spinlock);
        tasklet_schedule(tasklet);
}
```
