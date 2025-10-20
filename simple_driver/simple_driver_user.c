#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>  //open
#define SIZE 1024

int8_t write_buf[SIZE];
int8_t read_buf[SIZE];

int main()
{
    int fd;
    char options;
    printf("*****************\n");
    printf("*****thanhtu*****\n");
    printf("*****************\n");
    fd = open("/dev/simple_driver_df", O_RDWR);
    if(fd < 0)
    {
        printf("Cannot open file\n");
        return -1;
    }
    while(1)
    {
        printf("Enter option\n");
        printf("1. Write\n");
        printf("2. Read\n");
        printf("3. Exit\n");
        scanf("%c", &options);
        getchar();
        printf("Your options: %c\n", options);
        switch(options)
        {
            case '1':
                printf("Enter string to write into driver: ");
                fgets(write_buf, SIZE, stdin);
                printf("Data Writing ...");
                write(fd, write_buf, strlen(write_buf)+1);
                printf("Write done!\n");
                break;
            case '2':
                printf("Data reading....");
                read(fd, read_buf, SIZE);
                printf("Done!\n");
                printf("Data = %s\n", read_buf);
                break;
            case '3':
                close(fd);
                exit(1);
                break;
            default:
                printf("Enter Valid option = %c\n",options);
                break;
        }
    }
    close(fd);
    return 0;
}