/*
Week 5
LAB #02: time_server.c
이름: 박석현
학번: 2020110973
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_SIZE 100
#define TIME_REQ 0 // Request from client to server
#define TIME_RES 1 // Response from server to client

// Packet structure for communication
typedef struct {
    int cmd; // TIME_REQ or TIME_RES
    char time_msg[BUF_SIZE];
} PACKET;

void error_handling(char *message);
void set_time(PACKET *packet);

int main(int argc, char *argv[]) {
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    PACKET packet;

    // 포트번호 입력 여부 확인
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[1]);
        exit(1);
    }

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

    for (int i = 0; i < 5; i++) {
        // accept
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1) {
            error_handling("accept() error!");
        }
        printf("Connected client: %d, client_sock: %d\n", i + 1, clnt_sock);

        while (1) {
            memset(&packet, 0, sizeof(packet));

            int r_size = read(clnt_sock, &packet, sizeof(packet));
            if (r_size == -1) {
                error_handling("read error!");
            }
            else if (r_size == 0) {
                continue;
            }

            if (packet.cmd == TIME_REQ) {
                printf("[Server] Rx TIME_REQ\n");
                packet.cmd = TIME_RES;
                set_time(&packet);
                write(clnt_sock, &packet, sizeof(packet));
                printf("[Server] Rx time: %s\n", packet.time_msg);
            }
            else {
                printf("[Server] Rx Wrong cmd: %d\n", packet.cmd);
            }
        }
        close(clnt_sock);
    }

    close(serv_sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void set_time(PACKET *packet) {
    time_t t; time(&t);
    struct tm *p;
    p = localtime(&t);
    char res[50];
    // Year-Month-Day Hour:Minute:Second
    sprintf(res, "%d-%d-%d  %d:%d:%d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    strcpy((*packet).time_msg, res);
}
