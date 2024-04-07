/*
Week 4
HW #01: hw1_server.c
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
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    PACKET packet;

    // 포트번호 입력 여부 확인
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[1]);
        exit(1);
    }

    // start message
    printf("---------------------------\n");
    printf(" Address Conversion Server \n");
    printf("---------------------------\n");

    // socket
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        error_handling("socket() error!");
    }

    // 서버 IP주소, 포트번호 설정
    memset(&serv_addr, 0, sizeof(serv_addr));  // 초기화
    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // IP주소
    serv_addr.sin_port = htons(atoi(argv[1])); // 포트번호

    // bind
    if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("bind() error!");
    }

    // listen
    if (listen(serv_sock, 5) == -1) {
        error_handling("listen() error!");
    }

    // accept
    clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1) {
        error_handling("accept() error!");
    }

    while (1) {
        memset(&packet, 0, sizeof(packet)); // packet 초기화

        // read packet
        if (read(clnt_sock, &packet, sizeof(packet)) == 0) {
            break;
        }

        // REQUEST
        if (packet.cmd == REQUEST) {
            printf("[Rx] Received Dotted-Decimal Address: %s\n", packet.addr);

            // 주소가 정상인 경우
            if (inet_aton(packet.addr, &packet.iaddr)) {
                packet.cmd = RESPONSE;
                packet.result = SUCCESS;
                printf("inet_aton(%s) -> %#x\n", packet.addr, packet.iaddr.s_addr);
                printf("[Tx] cmd: %d, iaddr: %#x, result: %d\n\n", packet.cmd, packet.iaddr.s_addr, packet.result);
                if (write(clnt_sock, &packet, sizeof(packet)) == -1) {
                    error_handling("write() error!");
                }
            }
            
            // 잘못된 주소인 경우
            else {
                packet.cmd = RESPONSE;
                packet.result = ERROR;
                printf("[Tx] Address conversion fail:(%s)\n\n", packet.addr);
                if (write(clnt_sock, &packet, sizeof(packet)) == -1) {
                    error_handling("write() error!");
                }
            }
        }

        // QUIT
        else if (packet.cmd == QUIT) {
            printf("[Rx] QUIT message received\n");
            break;
        }

        // Others (Error)
        else {
            printf("[RX] Invalid command: %d\n", packet.cmd);
            break;
        }
    }

    printf("Server socket close and exit.\n");

    close(serv_sock);
    close(clnt_sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
