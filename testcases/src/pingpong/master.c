/* running on master computer, talk with slave computer, and record time */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define LEN 128
char* buf = "The quick brown fox jumps over the lazy dog 1234567890";
int iter = 20, loops =0;
char stopchar[1] = "\1";
struct timeval time_start, time_end;
double sum; // total time

void net_test(char* slave_ip){
    char buf_rcv[LEN];
    int udp_fd; // udp fd
    struct sockaddr_in s_addr; 

    // init udp
    udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_fd < 0){
        fprintf(stderr, "socket creating error.");
        exit(1);
    }
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(1234);
    s_addr.sin_addr.s_addr = inet_addr(slave_ip);
    
    gettimeofday(&time_start, NULL);
    for(int i = 0; i < iter; i++){
        sendto(udp_fd, buf, LEN, 0, (struct sockaddr*) &s_addr, sizeof(struct sockaddr));
        recv(udp_fd, buf_rcv, LEN, 0);
    }
    gettimeofday(&time_end, NULL);
    printf("send:%s \nrecv:%s\n", buf, buf_rcv);
    if (strcmp(buf, buf_rcv) == 0){ 
        printf("UDP test passed\n");
    }

    sendto(udp_fd, stopchar, 1, 0, (struct sockaddr *)&s_addr, sizeof(struct sockaddr));

    close(udp_fd);
}

void serial_test(char* serial_dev){
    char buf_rcv[LEN];
    char buf_tmp[LEN];
    int serial_fd;

    serial_fd = open(serial_dev, O_RDWR);
    if(serial_fd < 0){
        fprintf(stderr, "failed to open %s: %d\n", serial_dev, serial_fd);
        exit(-1);
    }

    memset(buf_rcv, 0, LEN);
    memset(buf_tmp, 0, LEN);
    iter = strlen(buf);

    gettimeofday(&time_start, NULL);
    for(int i=0; i<iter; i++){
        write(serial_fd, buf + i, 1);
        // printf("write: %c\n", buf[i]);
        
        do {
            read(serial_fd, buf_tmp, 1);
            loops++;
        } while(strlen(buf_tmp) < 1);
        memcpy(buf_rcv + i, buf_tmp, 1);
    }
    gettimeofday(&time_end, NULL);

    write(serial_fd, stopchar, 1);
    printf("send:%s\nrecv:%s\n", buf, buf_rcv);
    if(strcmp(buf, buf_rcv) == 0) printf("serial port test passed\n");

    close(serial_fd);
}

void pingpong_test(char* option[2]){
    int serial_fd, udp_fd;
    char *slave_ip = option[0];
    char *serial_dev = option[1];
    char buf_rcv[LEN];
    char buf_tmp[LEN];
    struct sockaddr_in s_addr; 

    serial_fd = open(serial_dev, O_RDWR | O_NONBLOCK);
    if(serial_fd < 0){
        fprintf(stderr, "failed to open %s: %d\n", serial_dev, serial_fd);
        exit(-1);
    }

    udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_fd < 0){
        printf("socket creating error.");
        exit(1);
    }
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(1234);
    s_addr.sin_addr.s_addr = inet_addr(slave_ip);

    gettimeofday(&time_start, NULL);
    for(int i = 0; i <= iter; i++){
        sendto(udp_fd, buf, LEN, 0, (struct sockaddr*) &s_addr, sizeof(struct sockaddr));
        do {
            read(serial_fd, buf_tmp, 1);
            loops++;
        } while(strlen(buf_tmp) < 1);
        memcpy(buf_rcv + i, buf_tmp, 1);
    }
    gettimeofday(&time_end, NULL);
    sendto(udp_fd, stopchar, 1, 0, (struct sockaddr*) &s_addr, sizeof(struct sockaddr));
    printf("send:%s\nrecv:%s\n", buf, buf_rcv);
    if(strcmp(buf, buf_rcv) == 0) printf("serial port test passed\n");

    close(udp_fd);
    close(serial_fd);
}

void print_result(void){
    sum = (time_end.tv_sec - time_start.tv_sec) * 1000000;
    // printf("total sec : %f\n", sum);
    sum += (time_end.tv_usec - time_start.tv_usec);
    // printf("total usec : %f\n", sum);
    printf("avg time(us) : %f\n", sum/2/(iter + loops)); 
}

void usage(void){
    fprintf(stderr, "Usage :\n\
      master -n(et) $slave_ip\n\
      master -s(erial) $serial_dev\n\
      master -p(ingpong) $slave_ip $serial_dev \n");
    exit(1);
}

int isoption(char* option, char* test){
    if(strcmp(option, test) == 0) return 1;
    if(option[0] != '-' && strlen(option) != 2) return 0;
    if(option[1] == test[1]) return 1;
    return 0;
}

int main(int argc, char* argv[]){
    if(argc == 3 && isoption(argv[1], "-net")) net_test(argv[2]);
    else if(argc == 3 && isoption(argv[1], "-serial")) serial_test(argv[2]);
    if(argc == 4 && isoption(argv[1], "-pingpong")) pingpong_test(argv+2);
    else usage();
    print_result();
}
