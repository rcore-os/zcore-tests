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
	int c, s;
	char buf[100];

	// tcp client
	TESTE((c=socket(PF_INET, SOCK_STREAM|SOCK_NONBLOCK, IPPROTO_TCP))>=0);
	TEST(fcntl(c, F_GETFL)&O_NONBLOCK, "SOCK_NONBLOCK did not work\n");
	// tpc connect addr and port
	sa.sin_addr.s_addr = htonl(0xC0A8007B); // 192.168.0.123
	//inet_aton( "192.168.0.123", &sa.sin_addr);
	sa.sin_port = htons(6979);

	// tcp connect 
	TESTE(connect(c, (void *)&sa, sizeof sa)==0 || errno==EINPROGRESS);
	printf("connect successfully\n");
	sleep(1);
	TESTE((s=sendto(c,"hello from TCP client\n!\n", 24, 0, (void *)&sa, sizeof sa))>=0);
	sleep(1);
	int rec_c = recvfrom(c, buf, sizeof buf, 0, (void *)&sa, (socklen_t[]){sizeof sa});
	for (int i = 0; i < rec_c; i++)
	{
		printf("%c", buf[i]);
	}
	close(c);
	return t_status;
}
