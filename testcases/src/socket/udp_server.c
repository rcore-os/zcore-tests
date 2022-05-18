// From libc-test/src/functional/socket.c

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <fcntl.h>
#include "test.h"

#define TEST(c, ...) ((c) ? 1 : (t_error(#c" failed: " __VA_ARGS__),0))
#define TESTE(c) (errno=0, TEST(c, "errno = %s\n", strerror(errno)))

int main(void)
{
	struct sockaddr_in sa = { .sin_family = AF_INET};
	// TEST()
	int s, len;
	char buf[100];

	TESTE((s=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))>=0);
	//sa.sin_addr.s_addr = htonl(0xC0A8007B); //192.168.0.123
	sa.sin_addr.s_addr = htonl(INADDR_ANY); // accept any addr from client
	sa.sin_port = htons(6969);              // local server port 6969

	TESTE(bind(s, (void *)&sa, sizeof sa)==0);
	TESTE(getsockname(s, (void *)&sa, (socklen_t[]){sizeof sa})==0);

	TESTE(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
		&(struct timeval){.tv_usec=1}, sizeof(struct timeval))==0);

	int open = 1;
	while (open) {
		len = recvfrom(s, buf, sizeof buf, 0, (void *)&sa, (socklen_t[]){sizeof sa});
		for (int i = 0 ; i < len;i++)
		{
			printf("%c", buf[i]);
			// TEST(buf[i]!='!', "'%c'\n", buf[i]);
			if (buf[i]=='*') {
				open = 0;
				sendto(s, "recv ok\n", 8, 0, (void *)&sa, sizeof sa);
			}
			if (buf[i] == '!') {
				printf("\n");
				sendto(s, "recv ok\n", 8, 0, (void *)&sa, sizeof sa);
				break;
			}
		}
	}
	sendto(s, "recv ok\n", 8, 0, (void *)&sa, sizeof sa);
	
	close(s);
	return t_status;
}

