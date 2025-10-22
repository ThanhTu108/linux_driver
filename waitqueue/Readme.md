# WaitQueue in Linux Device Driver
This function is used to put a process to sleep until some condition is true. 
Two types of wait queues are defined in the Linux kernel:
1. **Static Wait Queue**: 
- Define using `DECLARE_WAIT_QUEUE_HEAD(name)`
2. **Dynamic Wait Queue**:
- wait_queue_head_t name;
- initialize using `init_waitqueue_head(&name)`

## Queuing  
### 1. wait_event
```c
wait_event(name, condition);
```
Puts the process to sleep (TASK_UNINTERRUPTIBLE) until the condition is true. The condition is check each time the process is woken up.
### 2. wait_event_timeout
```c
wait_event_timeout(name, condition, timeout);
```
timeout : in jiffies <br>
Puts the process to sleep (TASK_UNINTERRUPTIBLE) until the condition is true after the timeout. Return 0 if the condition is false after timeout, otherwise return remaining jiffies if the condition to true before timeout.
### 3. wait_event_cmd
```c
wait_event_cmd(name, condition, cmd1, cmd2);
```
Puts the process to sleep (TASK_UNTINTERRUPTIBLE) until the condition is true. The condition is check each time the process is woken up. <br>
Execute cmd1 before sleep. <br>
Execute cmd2 after wake up.
### 4. wait_event_interruptible
```c
wait_event_interruptible(name, condition);
```
Puts the process to sleep (TASK_INTERRUPTIBLE) until the condition is true or a signal is received. The condition is check each time the process is woken up. <br>
Return 0 if the condition is true, otherwise return -ERESTARTSYS if a signal is received. (Ctrl+C by user / kill command).

### 5. wait_event_interruptible_timeout
```c
wait_event_interruptible_timeout(name, condition, timeout);
```
### 6. wait_event_killable
```c
wait_event_killable(name, condition);
```
Puts the process to sleep (TASK_KILLABLE) until the condition is true or a fatal signal is received. The condition is check each time the process is woken up.

## Waking up

### 1. wake_up
```c
wake_up(&name);
```
wakes up **ONLY** one processes from the wait queue which is non-interruptible sleep `name`.
### 2. wake_up_all
```c
wake_up_all(&name);
```
wakes up all process in the wait queue.

### 3. wake_up_interruptible
```c
wake_up_interruptible(&name);
```
wakes up all processes in the wait queue `name` which are in TASK_INTERRUPTIBLE state
### 4. wake_up_interruptible_sync
```c
wake_up_interruptible_sync(&name);
```
wakes up all processes in the wait queue `name` which are in TASK_INTERRUPTIBLE state and runs them on the CPU before returning.

## Usage 
1. cd driver/waitqueue
2. sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
3. sudo insmod static_waitqueue.ko / dynamic_waitqueue.ko
4. Check dmesg log using `dmesg` command
5. sudo su -> sudo cat /dev/static_wait_queue_devfile
6. echo "" > /dev/static_wait_queue_devfile
7. Check dmesg log using `dmesg` command 