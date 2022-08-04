CC := musl-gcc
CROSS_CC := riscv64-linux-musl-gcc

default : master udp uart ping-pong

master : master.c
	 $(CC) -o $@ $?
udp : udp.c
	$(CROSS_CC) -o $@ $?
uart : uart.c
	$(CROSS_CC) -o $@ $?
ping-pong : ping-pong.c
	$(CROSS_CC) -o $@ $?
clean :
	rm -f master udp uart ping-pong
