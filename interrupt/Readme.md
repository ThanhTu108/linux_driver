# Interrupt Driver
This is simple linux kernel module that demonstrates how to use interrupt handling in character device driver.

## Flow of interrupt driver
1. Define interrupt handler function
2. Request IRQ line using request_irq function
3. Free IRQ line using free_irq function
4. Implement module init and exit functions
## Usage
1. cd driver/interrupt
2. sudo make 
3. sudo insmod interrupt.ko
4. Trigger the interrupt (e.g., by cat the device file). (ex_interrupt is triggered on read operation and using top half handler to handle the interrupt)
5. check kernel log: dmesg
6. sudo rmmod interrupt

