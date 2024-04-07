/*
Week 4
HW #01: hw1_client.c
이름: 박석현
학번: 2020110973
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define ERROR 0
#define SUCCESS 1

#define BUF_SIZE 20

#define REQUEST 0
#define RESPONSE 1
#define QUIT 2

typedef struct {
    int cmd;  // 0: REQUEST, 1: RESPONSE, 2: QUIT
    char addr[BUF_SIZE];  // dotted-decimal address
    struct in_addr iaddr;  // inet_aton() result
    int result;  // 0: ERROR, 1: SUCCESS
} PACKET;

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;
    
    struct sockaddr_in serv_addr;

    PACKET packet;

    // IP주소, 포트번호 입력 여부 확인
    if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        error_handling("socket() error!");
    }

    // 서버의 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);  // IP주소
    serv_addr.sin_port = htons(atoi(argv[2]));  // 포트번호

    // connect
    if (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("connect() error!");
    }

    while (1) {
        memset(&packet, 0, sizeof(packet));  // pakcet 초기화

        printf("Input dotted-decimal address: ");
        scanf("%s", packet.addr);

        // 입력이 "quit"인 경우
        if (strcmp(packet.addr, "quit") == 0) {
            packet.cmd = QUIT;
            write(sock, &packet, sizeof(packet));
            printf("[Tx] cmd: %d(QUIT)\n", packet.cmd);
            break;
        }

        // 그 외 입력
        else {
            packet.cmd = REQUEST;
            write(sock, &packet, sizeof(packet));
            printf("[Tx] cmd: %d, addr: %s\n", packet.cmd, packet.addr);
        }

        // read packet
        if (read(sock, &packet, sizeof(packet)) == -1) {
            error_handling("read() error!");
        }

        // handle result
        if (packet.result == SUCCESS) {
            printf("[Rx] cmd: %d, Address conversoin: %#x (result: %d)\n\n", packet.cmd, packet.iaddr.s_addr, packet.result);
        }
        else if (packet.result == ERROR) {
            printf("[Rx] cmd: %d, Address conversoin fail! (result: %d)\n\n", packet.cmd, packet.result);
        }
        else {
            printf("[Rx] Invalid cmd: %d (result: %d)\n", packet.cmd, packet.result);
        }
    }

    printf("Client socket close and exit\n");

    close(sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
