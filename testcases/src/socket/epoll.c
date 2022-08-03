#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ssize_t socket_recv(int st)
{
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	ssize_t rc = recv(st, buf, sizeof(buf), 0);
	if (rc <= 0) {
		printf("recv failed %s\n", strerror(errno));
	} else {
		printf("recv %s\n", buf);
		send(st, buf, rc, 0);
	}
	return rc;
}

int socket_accept(int listen_st)
{
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    memset(&client_addr, 0, sizeof(client_addr));

    int client_st = accept(listen_st, (struct sockaddr *) &client_addr, &len);
    if (client_st < 0)
        printf("accept failed %s\n", strerror(errno));
    else
        printf("accepted %s\n", inet_ntoa(client_addr.sin_addr));
    return client_st;
}

void setnonblocking(int st) //将socket设置为非阻塞
{
    int opts = fcntl(st, F_GETFL);
    if (opts < 0)
    {
        printf("fcntl failed %s\n", strerror(errno));
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(st, F_SETFL, opts) < 0)
    {
        printf("fcntl failed %s\n", strerror(errno));
    }
}

int socket_create(int port)
{
    int st = socket(AF_INET, SOCK_STREAM, 0);
    int on = 1;
    if (setsockopt(st, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
    {
        printf("setsockopt failed %s\n", strerror(errno));
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    //addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(st, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        printf("bind port %d failed %s\n", port, strerror(errno));
        return -1;
    }
    if (listen(st, 300) == -1)
    {
        printf("listen failed %s\n", strerror(errno));
        return -1;
    }
    return st;
}

int main(int arg, char *args[])
{
    if (arg < 2){
	printf("please specify the PORT number with a parameter\n");
        return -1;
    }

    int iport = atoi(args[1]);    
    int listen_st = socket_create(iport);

    if (listen_st == -1)
        return -1;

    setnonblocking(listen_st); //把socket设置为非阻塞方式
    
    struct epoll_event ev, events[100]; //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
    int epfd = epoll_create(100); //生成用于处理accept的epoll专用的文件描述符

    ev.data.fd = listen_st; //设置与要处理的事件相关的文件描述符
    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP; //设置要处理的事件类型    
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_st, &ev); //注册epoll事件

    int st = 0;
    while (1)
    {
        int nfds = epoll_wait(epfd, events, 100, -1); //等待epoll事件的发生        
        if (nfds == -1)
        {
            printf("epoll_wait failed %s\n", strerror(errno));
            break;
        }
	printf("epoll return: %d\n", nfds);

        int i;
        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.fd < 0)
                continue;

            if (events[i].data.fd == listen_st) //监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            {
                st = socket_accept(listen_st);
                if (st >= 0)
                {
                    setnonblocking(st);
                    ev.data.fd = st;
                    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP; //设置要处理的事件类型    
                    epoll_ctl(epfd, EPOLL_CTL_ADD, st, &ev);
                    continue;
                }
            }
            if (events[i].events & EPOLLIN) //socket收到数据
            {
		printf("Event EPOLL IN\n");
                st = events[i].data.fd;
                if (socket_recv(st) <= 0)
                {
                    close(st);
                    events[i].data.fd = -1;
                }
            }
            if (events[i].events & EPOLLERR) //socket错误
            {
		printf("Event EPOLL ERR\n");
                close(st);
                events[i].data.fd = -1;
            }
            
            if (events[i].events & EPOLLHUP) //socket错误
            {
		printf("Event EPOLL HUP\n");
                close(st);
                events[i].data.fd = -1;
            }
        }
    }
    close(epfd);
    return 0;
}
