/*
Week 5
LAB #02: time_client.c
이름: 박석현
학번: 2020110973
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 100
#define TIME_REQ 0 // Request from client to server
#define TIME_RES 1 // Response from server to client

// Packet structure for communication
typedef struct {
    int cmd; // TIME_REQ or TIME_RES
    char time_msg[BUF_SIZE];
} PACKET;

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;
    
    struct sockaddr_in serv_addr;

    PACKET packet;

    // buf
    char buf[BUF_SIZE];

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
    
    printf("Connected..........\n");

    while (1) {
        memset(&packet, 0, sizeof(packet));

        printf("Type a message(time or q): ");
        scanf("%s", buf);

        if (strcmp(buf, "time") == 0) {
            packet.cmd = TIME_REQ;
            if (write(sock, &packet, sizeof(packet)) == -1) {
                error_handling("write error!");
            }

            if (read(sock, &packet, sizeof(packet)) == -1) {
            error_handling("read error!");
            }

            if (packet.cmd == TIME_RES) {
                printf("[Client] Rx TIME_RES: %s\n", packet.time_msg);
            }

            else {
                printf("[Client] Rx Wrong cmd: %d\n", packet.cmd);
                break;
            }
        }

        else if (strcmp(buf, "q") == 0) {
            printf("Exit client\n");
            break;
        }

        else {
            printf("Wrong message.\n");
        }
    }

    close(sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
