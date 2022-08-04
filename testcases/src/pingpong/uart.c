#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void usage(void){
    fprintf(stderr, "Usage :\n  uart $serial_dev\n");
    exit(1);
}

/* read from uart, write to uart */
int main(int argc, char* argv[]){
    char *uart_name;
    int fd, len, pos;
    char buf_rcv[128];

    if (argc != 2){
        usage();
    }
    uart_name = argv[1];

    fd = open(uart_name, O_RDWR);
    if (fd < 0) {
        printf("failed to open %s: %d\n", uart_name, fd);
        return -1;
    }
    
    while (1)
    {
        memset(buf_rcv, 0, 128);
        while(strlen(buf_rcv) < 1){
            read(fd, buf_rcv, 1);
        }
        if(buf_rcv[0] == '\1') break;
        write(fd, buf_rcv, 1);
        // printf("write : %s\n", buf_rcv);
    }
    close(fd);
}
