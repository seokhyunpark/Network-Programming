/*
HW #02: view_server.c
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
    int serv_sock;
    int clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    // variables
    int bytes;
    int tx_count;
    int r_len;
    int fd;
    char file_name[BUF_SIZE];
    PACKET send_packet;
    PACKET recv_packet;

    // 포트번호 입력 여부 확인
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[1]);
        exit(1);
    }

    printf("---------------------------\n");
    printf("TCP Remote File View Server\n");
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
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1) {
        error_handling("accept() error!");
    }

    bytes = 0;
    tx_count = 0;

    init_packet(&recv_packet);
    read(clnt_sock, file_name, sizeof(file_name));  // 파일이름 수신
    read(clnt_sock, &recv_packet, sizeof(recv_packet));  // 패킷 수신
    printf("[Rx] cmd: %d, file_name: %s\n", recv_packet.cmd, file_name);

    fd = open(file_name, O_RDONLY);  // open file
    // open() 실패
    if (fd == -1) {
        init_packet(&send_packet);
        send_packet.cmd = FILE_NOT_FOUND;
        write(clnt_sock, &send_packet, sizeof(send_packet));
        printf("[Tx] cmd: %d, %s: File Not Found\n", send_packet.cmd, file_name);
    }
    // open() 성공
    else {
        while (1) {
            init_packet(&send_packet);
            r_len = read(fd, send_packet.buf, sizeof(send_packet.buf));  // BUF_SIZE만큼 읽기
            send_packet.buf_len = r_len;

            bytes += send_packet.buf_len;
            tx_count++;

            if (r_len < BUF_SIZE) {  // 파일의 마지막 부분
                send_packet.cmd = FILE_END;
                write(clnt_sock, &send_packet, sizeof(send_packet));
                printf("[Tx] cmd: %d, len: %3d, total_tx_cnt: %3d, total_tx_bytes: %d\n", send_packet.cmd, send_packet.buf_len, tx_count, bytes);
                break;
            }
            else {  // 파일의 마지막 부분이 아닌 경우
                send_packet.cmd = FILE_RES;
                write(clnt_sock, &send_packet, sizeof(send_packet));
                printf("[Tx] cmd: %d, len: %3d, total_tx_cnt: %3d, total_tx_bytes: %d\n", send_packet.cmd, send_packet.buf_len, tx_count, bytes);
            }
            sleep(1);
        }
        init_packet(&recv_packet);
        read(clnt_sock, &recv_packet, sizeof(recv_packet));  // try to read FILE_END_ACK
        if (recv_packet.cmd == FILE_END_ACK) {
            printf("[Rx] cmd: %d, FILE_END_ACK\n", recv_packet.cmd);
        }
        else {
            printf("Invalid cmd: %d\n", recv_packet.cmd);
            exit(1);
        }
    }

    printf("----------------------------------------\n");
    printf("Total Tx count: %d, bytes: %d\n", tx_count, bytes);
    printf("TCP Server Socket Close!\n");
    printf("----------------------------------------\n");

    close(fd);
    close(serv_sock);
    close(clnt_sock);
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
