#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>

/* receive from udp, write to uart */
int main(int argc, char* argv[]){
    char *serial_name;
    int udp, uart;    // udp fd, uart fd
    struct sockaddr_in sa;
    char buf[128];

    if(argc != 2){
        fprintf(stderr, "Usage:\n ping-pong $serial_dev\n");
        return 1;
    }
    serial_name = argv[1];

    // init udp
    udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp < 0)    {
        fprintf(stderr, "failed to open udp socket: %s\n", strerror(errno));
        return 1;
    }
    printf("socket = %d\n", udp);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(1234);
    if (bind(udp, (void *)&sa, sizeof sa) < 0)    {
        printf("failed to bind: %s\n", strerror(errno));
        return 1;
    }

    // init uart
    uart = open(serial_name, O_RDWR | O_NONBLOCK);
    if (uart < 0)    {
        printf("failed to open %s: %s\n", serial_name, strerror(errno));
        return 1;
    }
    printf("uart = %d\n", udp);

    // work
    while (1) {
        memset(buf, 0, sizeof(buf));
        unsigned long len = read(udp, buf, sizeof(buf));
        if (len == 1 && *buf == '\1'){
            printf("over.\n");
            break;
        }
        if (len > 0){
            buf[len] = '\0';
            printf("received: %s\n", buf);
            write(uart, buf, len);
        }
    }
}
