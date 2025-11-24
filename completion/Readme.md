# Completion 
Completion used to notify or wake up some thread, do something when we finish some work.  

```c
struct completion 
{
    unsigned int done;
    struct swait_queue_head head;
};
```
**There are 5 step in completions**
1. Initialize completion
2. Re-initialize completion
3. Waiting for completion 
4. Wake up task (call sleep task)
5. Check status  

## 1. Initialize
**Static**
```c
DECLARE_COMPLETION(name);
```
**Dynamic**
```c
init_completion(struct completion* name);
```

Both static and dynamic methods result in done = 0, it mean "not completed" or "not done".  
Ex:
```c
struct completion dynamic_completion;
init_completion(&dynamic_completion);
```

## 2. Re-initialize
This function is used to reset completion to "not completed, not done", done = 0 for re use.  
```c
reinit_completion(struct completion* name);
```

### 3. Waiting for completion  

**wait_for_completion**

This function used to put the thread into state sleep and wake up when completion of the task  
```c
void wait_for_completion(struct completion* x);
```
This function is call spin_lock_irq and spin_unlock_irq when done  
  
**Wait_for_completion_timerout(struct completion* x, unsigned long timeout)**

**Wait_for_completion_interrupts**

## 4. Waking up

**Complete**
This function will wake up a single thread waiting on this completion.
```c
void complete(struct completion* x);
```

**Complete all**
This function will wake up all thread waiting on this completion.
```c
void complete_all(struct completion* x);
```

## 5. Check the status
```c
bool completion_done (struct completion * x);
```
It return 0 if there are waiters (wait_for_completion() in progress) 1 if there are no waiters.  
