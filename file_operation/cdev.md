# lib cdev.h
```c
#include <linux/kobject.h>
#include <linux/kdev_t.h>
#include <linux/list.h>
#include <linux/device.h>
struct file_operations;
struct inode;
struct module;
struct cdev {
	struct kobject kobj;
	struct module *owner;
	const struct file_operations *ops;
	struct list_head list;
	dev_t dev;
	unsigned int count;
} __randomize_layout;

void cdev_init(struct cdev *, const struct file_operations *);
struct cdev *cdev_alloc(void);
void cdev_put(struct cdev *p);
int cdev_add(struct cdev *, dev_t, unsigned);
void cdev_set_parent(struct cdev *p, struct kobject *kobj);
int cdev_device_add(struct cdev *cdev, struct device *dev);
void cdev_device_del(struct cdev *cdev, struct device *dev);
void cdev_del(struct cdev *);
void cd_forget(struct inode *);
```


# Another function in cdev.c
If you wish to obtain a standalone cdev structure at runtime, you may do so with code such as:
```c
struct cdev *my_cdev = cdev_alloc();	#create cdev struct pointer and allocate memory
my_cdev->ops = &my_fops;	#assign file operations to cdev
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
{
	memset(cdev, 0, sizeof *cdev);
	INIT_LIST_HEAD(&cdev->list);
	kobject_init(&cdev->kobj, &ktype_cdev_default);
	cdev->ops = fops;
}

```