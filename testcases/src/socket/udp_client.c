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
	// udp client
	TESTE((c=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))>=0);
	// udp connect addr and port
	sa.sin_addr.s_addr = htonl(0xC0A8007B);
	sa.sin_port = htons(6969);
	TESTE((s=sendto(c,"Hello from UDP client\n!\n", 24, 0, (void *)&sa, sizeof sa))>=0);
	int rec_c = recvfrom(c, buf, sizeof buf, 0, (void *)&sa, (socklen_t[]){sizeof sa});
	printf("received length: %d\n", rec_c);
	for (int i = 0 ; i < rec_c; i++)
	{
		printf("%c", buf[i]);
	}
	close(c);
	return t_status;
}
