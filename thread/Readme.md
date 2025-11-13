# Kernel thread (Kthread)
Kernel threads are created and managed by the OS kernel.They run in kernel space.

## ** Kernel thread management functions: **
### 1. Create a kernel thread
This API creates a kernel thread and returns a pointer to the task_struct structure of the created thread.
```c
struct task_struct* kthread_create(int (*threadfn)(void* data), void* data, const char namefmt[], ...);
```
- `threadfn`: The function that the kernel thread will execute.
- `data`: The argument passed to the thread function.
- `namefmt`: The name of the kernel thread.
This helper function only creates the thread, it does not start it. To start the thread, use the `wake_up_process` function. When wake up is called, the thread will run threadfn() immediately.
### 2. Start a kernel thread
This API wakes up a kernel thread, allowing it to start executing.
```c
void wake_up_process(struct task_struct* p);
```
- `p`: The process to be woken up.
Return 1 if the process was successfully woken up, otherwise return 0.

### 3. Stop kernel thread
It stop a thread that was created with kthread_create.
```c
int kthread_stop(struct task_struct* k);
```
- `k`: The kernel thread to be stopped.
Sets kthread_should_stop() for k to return true, wakes it up if it is sleeping, and waits for it to exit. Your threadfn must not call do_exit() itself if you want to use kthread_stop(). It Returns the result of threadfn, or –EINTR if wake_up_process was never called.

### 4. Other function in kernel thread
```c
int kthread_should_stop(void);
```
When someone calls kthread_stop() on your thread. It will be wake up if it is sleeping and return true. You should then return and your return value will be passed back to the kthread_stop() caller.


## Implementation
1. Create thread function my_threadfn.
```c
int my_threadfn(void* data)
{
    while(!kthread_should_stop())
    {
        pr_info("Kernel thread is running...\n");
        ssleep(1); 
    }
    pr_info("Kernel thread is stopping...\n");
    return 0;
}
```
2. Create and start kernel thread 
You can create and start in 2 ways:
- Method 1: Create and start separately
```c
my_kthread = kthread_create(my_threadfn, NULL, "my_kthread");
if(my_kthread)
{
    wake_up_process(my_kthread);
}
else
{
    pr_err("Failed to create kernel thread\n");
}
```
- Method 2: Create and start together
```c
my_kthread = kthread_run(my_threadfn, NULL, "my_kthread");
if(IS_ERR(my_kthread))
{
    pr_err("Failed to create and start kernel thread\n");
}
```
3. Stop kernel thread
```c
if(my_kthread)
{
    kthread_stop(my_kthread);
}
```