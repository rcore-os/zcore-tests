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
	int sock, s, fl;
	char buf[100];
	// udp client
	TESTE((sock=socket(PF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP))>=0);
	//TESTE((sock=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))>=0); //默认阻塞

	// O_NONBLOCK 一般会配合poll检测多个fd状态
	TEST((fl=fcntl(sock, F_GETFL)) & O_NONBLOCK, "SOCK_NONBLOCK did not work\n");

	if (fl != -1){
		if (fcntl(sock, F_SETFL,fl & !O_NONBLOCK) != -1)
			printf("fcntl SETFL succeed\n");
	}
	printf("init socket flags: %#x, %#x\n", fcntl(sock, F_GETFD), fcntl(sock, F_GETFL));

	// udp connect addr and port
	// sa.sin_addr.s_addr = htonl(0xC0A8007B);
	sa.sin_addr.s_addr = inet_addr("192.168.0.62");
	sa.sin_port = htons(6969);
	TESTE((s=sendto(sock,"Hello from UDP client\n!\n", 24, 0, (void *)&sa, sizeof sa))>=0);
	printf("send length: %d\n", s);
	int rec_c = recvfrom(sock, buf, sizeof buf, 0, (void *)&sa, (socklen_t[]){sizeof sa});
	printf("received length: %d\n", rec_c);
	for (int i = 0 ; i < rec_c; i++)
	{
		printf("%c", buf[i]);
	}
	close(sock);
	return t_status;
}
