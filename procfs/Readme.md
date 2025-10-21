# Procfs
Procfs (Process Filesystem) is a virtual filesystem in Linux that provides an interface to kernel data structures. It is commonly used to obtain information about processes and other system information.
This directory is used to create a simple procfs driver example in Linux kernel module programming.

## Features
- Create a procfs entry under `/proc` directory.
- Implement read and write operations for the procfs entry.
- Demonstrate how to interact with user space through procfs.
## Usage
1. Compile the kernel module using the provided Makefile.
2. Load the module into the kernel using `sudo insmod ex_procfs.ko`.
3. Access the procfs entry using `cat /ex_procfs/procfs_1` to read data and `echo "your_data" > /ex_procfs/procfs_1` to write data.