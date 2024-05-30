/*
Week 11
HW #06: hw06_client.c
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
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 2048
#define SENDER 1
#define RECEIVER 2

void error_handling(char *message);

int main(int argc, char *argv[]) {
	char message[BUF_SIZE];
	struct sockaddr_in serv_adr;

    int role;
    int fd1;
    int fd2;

    struct timeval timeout;
	fd_set reads, cpy_reads;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];

	if(argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

    printf("------------------------------------\n");
    printf("Choose function\n");
    printf("1. Sender,   2. Receiver\n");
    printf("------------------------------------\n");
    printf(" => ");
    scanf("%d", &role);

    if (role == SENDER) {
        fd1 = open("rfc1180.txt", O_RDONLY);
        if (fd1 == -1) {
            printf("rfc1180.txt doesn't exist.\n");
            exit(1);
        }
    }

    fd2 = socket(PF_INET, SOCK_STREAM, 0);
	if(fd2 == -1) {
        error_handling("socket() error");
    }

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));
	
	if(connect(fd2, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1) {
        error_handling("connect() error!");
    }
	else {
        puts("");
        puts("File Receiver Start!");
        puts("Connected...........");
    }

    if (role == SENDER) {
        FD_ZERO(&reads);
        FD_SET(fd1, &reads);
        FD_SET(fd2, &reads);
        fd_max = fd2;
        printf("fd1: %d, fd2: %d\n", fd1, fd2);
        printf("max_fd: %d\n", fd_max);
    }
    else if (role == RECEIVER) {
        FD_ZERO(&reads);
        FD_SET(fd2, &reads);
        fd_max = fd2;
        printf("fd2: %d\n", fd2);
        printf("max_fd: %d\n", fd_max);
    }
    else {
        printf("Invalid Input.\n");
        exit(1);
    }

    while (1) {
        memset(buf, 0, BUF_SIZE);

        cpy_reads=reads;
		timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        if((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1) {
            break;
        }
		if(fd_num == 0) {
            continue;
        }

        if (role == SENDER) {
            if (FD_ISSET(fd1, &cpy_reads)) {
                str_len = read(fd1, buf, BUF_SIZE);
                if (str_len == 0) {
                    FD_CLR(fd1, &reads);
                    close(fd1);
                }
                else {
                    write(fd2, buf, str_len);
                    sleep(1);
                }
            }
            if (FD_ISSET(fd2, &cpy_reads)) {
                read(fd2, buf, BUF_SIZE);
                printf("%s", buf);
            }
        }
        else if (role == RECEIVER) {
            if (FD_ISSET(fd2, &cpy_reads)) {
                str_len = read(fd2, buf, BUF_SIZE);
                if (str_len == 0) {
                    FD_CLR(fd2, &reads);
                    close(fd2);
                }
                else {
                    printf("%s", buf);
                    write(fd2, buf, BUF_SIZE);
                }
            }
        }
    }

	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
