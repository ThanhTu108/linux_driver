#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>

#define TASK_CURRENT _IOW('a', 'a', int32_t*)
#define SIGTX 44

static int done = 0;
int check = 0;

void ctrl_c_handler(int n, siginfo_t* info, void* unsued)
{
    if(n == SIGINT)
    {
        printf("Ctrl c\n");
        done = 1;
    }
}

void sig_event_handler(int n, siginfo_t* info, void* unsued)
{
    if(n == SIGTX)
    {
        check = info->si_int;
        printf("Receive signal from kernal value = %u\n", check);
    }
}


int main()
{
    int fd;
    int32_t value, number;
    struct sigaction act;
    printf("....................\n");
    printf("......ThanhTu.......\n");
    printf("....................\n");
    
    //install ctrc c
    sigemptyset(&act.sa_mask);
    act.sa_flags = (SA_SIGINFO | SA_RESETHAND);
    //void   (*sa_sigaction)(int, siginfo_t *, void *)  function pointer sig actions
    act.sa_sigaction = ctrl_c_handler;
    //register sig handler
    sigaction(SIGINT, &act, NULL);


    sigemptyset(&act.sa_mask);
    act.sa_flags = (SA_SIGINFO | SA_RESETHAND);
    //void   (*sa_sigaction)(int, siginfo_t *, void *)  function pointer sig actions
    act.sa_sigaction = sig_event_handler;
    //register sig handler
    sigaction(SIGTX, &act, NULL);

    printf("Register signal handler ctrc + c and SIGTX = %d\n", SIGTX);

    printf("Open device file\n");
    fd = open("/dev/send_sig_device", O_RDWR);
    if(fd < 0)
    {
        printf("Can not open device file\n");
        return -1;
    }

    //ioctl 
    if(ioctl(fd, TASK_CURRENT, (int32_t*) &number))
    {
        printf("Fail\n");
        close(fd);
        exit(1);
    }
    printf("Done!!!\n");
   
    while(!done) {
        printf("Waiting for signal...\n");
 
        //blocking check
        while (!done && !check);
        check = 0;
    }
    printf("Closing Driver\n");
    close(fd);
}