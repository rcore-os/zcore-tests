#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int fd;
    char buf[1024];
    struct sockaddr_in sa_to, sa_from;
    socklen_t size = sizeof(sa_to);

	/* read from udp */
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (fd < 0)
    {
        printf("failed to open socket: %s\n", strerror(errno));
        return 1;
    }

    printf("socket = %d\n", fd);

    sa_to.sin_family = AF_INET;
    sa_to.sin_port = htons(1234);
    if (bind(fd, (void *)&sa_to, size) < 0)
    {
        printf("failed to bind: %s\n", strerror(errno));
        return 1;
    }

    printf("bind done\n");

    if (getsockname(fd, (void *)&sa_to, &size) < 0)
    {
        printf("failed to getsockname: %s\n", strerror(errno));
        return 1;
    }

    printf("socket bind to %d\n", ntohs(sa_to.sin_port));

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int len = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *) &sa_from, &size);
        if (len == 1 && *buf == '\1')
        {
            printf("over.\n");
            break;
        }
        else if (len > 0)
        {
            buf[len] = '\0';
            // printf("received: %s\n", buf);
            sendto(fd, buf, len, 0, (struct sockaddr*) &sa_from, sizeof(sa_from));
        }
    }
    close(fd);
}
