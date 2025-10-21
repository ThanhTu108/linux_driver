#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#define WR_VAL _IOW('a', 'a', int32_t* )
#define RD_VAL _IOR('a', 'b', int32_t* )

int main()
{
    int fd;
    int32_t value_send;
    int32_t value_recv;
    fd = open("/dev/ioctl_device", O_RDWR);
    if(fd == 0)
    {
        printf("Err open device file");
        return -1;
    }
    printf("Enter value to send: ");
    scanf("%d", &value_send);
    ioctl(fd, WR_VAL, (int32_t*)&value_send);


    printf("Reading data from kernel\n");
    ioctl(fd, RD_VAL, (int32_t*)&value_recv);
    printf("Value receive = %d\n", value_recv);
    printf("Close\n");
    close(fd);
    return 0;
}