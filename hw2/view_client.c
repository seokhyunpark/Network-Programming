/*
HW #02: view_client.c
이름: 박석현
학번: 2020110973
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 100

// cmd type
#define FILE_REQ 1
#define FILE_RES 2
#define FILE_END 3
#define FILE_END_ACK 4
#define FILE_NOT_FOUND 5

typedef struct {
    int cmd;
    int buf_len;  // 실제 전송되는 파일의 크기 저장
    char buf[BUF_SIZE];
} PACKET;

void error_handling(char *message);
void init_packet(PACKET *packet);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;

    // variables
    int rx_count;
    int bytes;
    char file_name[BUF_SIZE];
    PACKET send_packet;
    PACKET recv_packet;

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

    // 파일이름 입력 받기
    printf("Input file name: ");
    scanf("%s", file_name);

    init_packet(&send_packet);
    send_packet.cmd = FILE_REQ;
    write(sock, file_name, sizeof(file_name));  // 파일이름 전송
    write(sock, &send_packet, sizeof(send_packet));  // 패킷 전송
    printf("[Tx] cmd: %d, file name: %s\n", send_packet.cmd, file_name);

    bytes = 0;
    rx_count = 0;

    while (1) {
        init_packet(&recv_packet);
        read(sock, &recv_packet, sizeof(recv_packet));
        bytes += recv_packet.buf_len;

        if (recv_packet.cmd == FILE_NOT_FOUND) {
            printf("[Rx] cmd: %d, %s: File Not Found\n", recv_packet.cmd, file_name);
            break;
        }
        else if (recv_packet.cmd == FILE_END) {
            printf("%s", recv_packet.buf);
            printf("\n---------------------------\n");
            printf("[Rx] cmd: %d, FILE_END\n", recv_packet.cmd);

            init_packet(&send_packet);
            send_packet.cmd = FILE_END_ACK;
            write(sock, &send_packet, sizeof(send_packet));
            printf("[Tx] cmd: %d, FILE_END_ACK\n", send_packet.cmd);
            rx_count++;
            break;
        }
        else if (recv_packet.cmd == FILE_RES) {
            printf("%s", recv_packet.buf);
            rx_count++;
        }
        else {
            printf("[Rx] Invalid cmd: %d\n", recv_packet.cmd);
            exit(1);
        }
    }
    
    printf("------------------------------------\n");
    printf("Total Rx count: %d, bytes: %d\n", rx_count, bytes);
    printf("TCP Client Socket Close!\n");
    printf("------------------------------------\n");

    close(sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void init_packet(PACKET *packet) {
    memset(packet, 0, sizeof(*packet));
}
