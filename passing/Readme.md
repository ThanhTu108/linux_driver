# Passing Data Between Kernel and User Space

This is a simple Linux kernel module that demonstrates how to pass data between the kernel and user space using module parameters. The module allows you to pass an integer, an array of integers, and a string from user space to the kernel. Additionally, it includes a callback function that is triggered when the integer parameter is set.

## Features
- Pass an integer parameter (`val`) from user space to the kernel.
- Pass an array of integers (`arr_val`) from user space to the kernel.
- Pass a string parameter (`name`) from user space to the kernel.
- Callback function (`my_param_set_int`) that is called when the integer parameter is set,

    printing the new value to the kernel log.
- Module parameters are set with read and write permissions for the user.
## Usage
1. Compile the kernel module using `sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-`.
2. Load the module into the kernel using `sudo insmod passing.ko` val=<integer> arr_val = array_of_integers name=<string>.
3. Check module_param_cb:
    - sudo su
    echo <integer> > /sys/module/passing/parameters/cb_value
4. Check the kernel log to see the output of the module using `dmesg`.
5. Unload the module from the kernel using `sudo rmmod passing`.

## Another struct in module_param
struct kernel_param {
	const char *name;
	struct module *mod;
	const struct kernel_param_ops *ops;
	const u16 perm;
	s8 level;
	u8 flags;
	union {
		void *arg;
		const struct kparam_string *str;
		const struct kparam_array *arr;
	};
};

struct kernel_param_ops {
    int (*set)(const char *val, const struct kernel_param *kp);
    int (*get)(char *buffer, const struct kernel_param *kp);
    void (*free)(const struct kernel_param *kp);
};

