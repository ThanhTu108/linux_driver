# Mutex
A mutex (mutual exclusion). Only one thread can hold the lock at a time. It avoid race conditions when multiple threads access a memory location concurrently.  
One mutex can be locked by only thread at a time. If a thread tries to lock a mutex that is already locked by another thread, it will be put to sleep until the mutex becomes available.  
So whenever you are accessing a shared resource that time first lock the mutex and after accessing unlock the mutex.  
## Functions
This is struct mutex definition:
```c
struct mutex 
{
    atomic_t count;
    wait_queue_head_t wait_lock;
    struct list_head wait_list;
};
```
There are two ways to initialize a mutex:
```c
#include <linux/mutex.h>
```
1. Static method:
This method is used when using mutex in global scope.  
```c
DEFINE_MUTEX(name);
```
2. Dynamic method:
This method is used when using mutex in local scope.  
```c
struct mutex name;
mutex_init(&name);
```
### Locking and Unlocking
To lock a mutex, use there functions:

**Mutex lock**

1. Mutex_lock:
This function is used to lock the mutex for the current thread. If the mutex is already locked by another thread, the current thread will be put to sleep until the mutex becomes available.
```c
void mutex_lock(struct mutex* lock);
```
2. Mutex_lock_interruptible:
This function is similar to mutex_lock, but it allows the thread to be interrupted by signals while waiting for the mutex to become available.
```c
int mutex_lock_interruptible(struct mutex* lock);
```

3. Mutex_trylock:
This function attempts to lock the mutex without blocking but not sleep. Return 0 if the lock is accquired successfully, 0 if not.
```c
int mutex_trylock(struct mutex* lock);
```

**Mutex unlock**
To unlock a mutex, use function: mutex_unlock. This used to unlock or release the mutex held by the current thread.
```c
void mutex_unlock(struct mutex* lock);
```

**Checking mutex status**
This function is used to check whether the mutex has been locked or not.
```c
int mutex_is_locked(struct mutex* lock);
```



