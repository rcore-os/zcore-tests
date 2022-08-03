#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<poll.h>

static void usage(const char *proc)
{
	printf("%s [local_ip] [local_port]\n",proc);
}
int start_up(const char*_ip,int _port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0); // TCP
	if(sock < 0)
	{
		perror("socket");
		return 2;
	}
	int opt = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(_port);
	local.sin_addr.s_addr = inet_addr(_ip);
	if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
	{
		perror("bind");
		return 3;
	}
	if(listen(sock,10) < 0)
	{
		perror("listen");
		return 4;
	}
	return sock;
}

int main(int argc, char*argv[])
{
	if(argc != 3)
	{
		usage(argv[0]);
		return 1;
	}

	int sock = start_up(argv[1],atoi(argv[2]));

	struct pollfd peerfd[1024];
	peerfd[0].fd = sock;
	peerfd[0].events = POLLIN;

	int nfds = 1;
	int ret;
	int maxsize = sizeof(peerfd)/sizeof(peerfd[0]);
	int i = 1;

	int timeout = -1;
	//int timeout = 200;

	for(; i < maxsize; ++i)
	{
		peerfd[i].fd = -1;
	}
	while(1)
	{
		printf("+++++++++ start poll nfds: %d\n", nfds);
		switch(ret = poll(peerfd,nfds,timeout))
		{
			case 0:
				printf("timeout...\n");
				break;
			case -1:
				perror("poll");
				break;
			default:
				{
					if(peerfd[0].revents & POLLIN)
					{
						struct sockaddr_in client;
						socklen_t len = sizeof(client);
						int new_sock = accept(sock,\
								(struct sockaddr*)&client,&len);
						printf("accept finish %d\n",new_sock);
						if(new_sock < 0)
						{
							perror("accept");
							continue;
						}
						printf("0 get a new client\n");
						int j = 1;
						for(; j < maxsize; ++j)
						{
							if(peerfd[j].fd < 0)
							{
								peerfd[j].fd = new_sock;
								break;
							}
						}
						if(j == maxsize)
						{
							printf("to many clients...\n");
							close(new_sock);
						}
						peerfd[j].events = POLLIN;
						if(j + 1 > nfds)
							nfds = j + 1;
					}

					for(i = 1;i < nfds;++i)
					{
						printf("--------- revent %d: %#x\n", i, peerfd[i].revents);
						if(peerfd[i].revents & POLLIN)
						{
							printf("%d read ready\n", i);
							char buf[1024];
							ssize_t s = read(peerfd[i].fd,buf, \
									sizeof(buf) - 1);
							if(s > 0)
							{
								buf[s] = 0;
								printf("client say # {%s}\n",buf);
								fflush(stdout);
								peerfd[i].events = POLLOUT;
							}
							else if(s <= 0)
							{
								close(peerfd[i].fd);
								peerfd[i].fd = -1;
							}
							else
							{

							}
						}//i != 0
						else if(peerfd[i].revents & POLLOUT)
						{
							printf("%d write something out\n", i);
							char *msg = "HTTP/1.0 200 OK \r\n\r\n<html><h1>Hello World from XLY</h1></html>\r\n";
							write(peerfd[i].fd,msg,strlen(msg));
							close(peerfd[i].fd);
							peerfd[i].fd = -1;
						}
						else
						{
						}
					}//for
				}//default
				break;
		}
	}
	return 0;
}
