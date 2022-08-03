// From libc-test/src/functional/socket.c

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>

#define T_LOC2(l) __FILE__ ":" #l
#define T_LOC1(l) T_LOC2(l)
#define t_error(...) t_printf(T_LOC1(__LINE__) ": " __VA_ARGS__)

#define TEST(c, ...) ((c) ? 1 : (t_error(#c" failed: " __VA_ARGS__),0))
#define TESTE(c) (errno=0, TEST(c, "errno = %s\n", strerror(errno)))

volatile int t_status = 0;

int t_printf(const char *s, ...)
{
	va_list ap;
	char buf[512];
	int n;

	t_status = 1;
	va_start(ap, s);
	n = vsnprintf(buf, sizeof buf, s, ap);
	va_end(ap);
	if (n < 0)
		n = 0;
	else if (n >= sizeof buf) {
		n = sizeof buf;
		buf[n - 1] = '\n';
		buf[n - 2] = '.';
		buf[n - 3] = '.';
		buf[n - 4] = '.';
	}
	return write(1, buf, n);
}

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

	//设置接收超时
	TESTE(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
		&(struct timeval){.tv_sec=1, .tv_usec=1000}, sizeof(struct timeval))==0);

	printf("socket flags: %#x, %#x\n", fcntl(s, F_GETFD), fcntl(s, F_GETFL));
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

