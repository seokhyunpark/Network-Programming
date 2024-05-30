/*
Week 11
HW #06: hw06_server.c
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

void error_handling(char *message);

int main(int argc, char *argv[]) {
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	struct timeval timeout;
	fd_set reads, cpy_reads;

	socklen_t adr_sz;
	int fd_max, str_len, fd_num, i;
	char buf[BUF_SIZE];

    int send_sock = 0;
    int recv_sock = 0;

	if(argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1) {
        error_handling("socket() error");
    }

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("bind() error");
    }
	if(listen(serv_sock, 5) == -1) {
        error_handling("listen() error");
    }

	FD_ZERO(&reads);
	FD_SET(serv_sock, &reads);
	fd_max = serv_sock;

	while(1) {
		memset(buf, 0, BUF_SIZE);

		cpy_reads=reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if((fd_num=select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1) {
            break;
        }
		if(fd_num == 0) {
            continue;
        }

		for(i = 0; i < fd_max + 1; i++) {
			if (FD_ISSET(i, &cpy_reads)) {
				if (i == serv_sock) {
					adr_sz = sizeof(clnt_adr);
					clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
					FD_SET(clnt_sock, &reads);
					if (fd_max < clnt_sock) {
                        fd_max = clnt_sock;
                    }
					printf("connected client: %d (fd_max: %d)\n", clnt_sock, fd_max);

                    if (recv_sock == 0) {
                        recv_sock = clnt_sock;
                    }
                    else if (send_sock == 0) {
                        send_sock = clnt_sock;
                    }
				}
				else if (i == send_sock) {
                    str_len = read(send_sock, buf, BUF_SIZE);
					if (str_len == 0) {
						FD_CLR(send_sock, &reads);
						close(send_sock);
						printf("closed client: %d \n", i);
					}
					else {
						write(recv_sock, buf, str_len);
						printf("Forward  [%d] ---> [%d]\n", send_sock, recv_sock);
                        printf("Backward [%d] <--- [%d]\n", send_sock, recv_sock);
					}				
				}
                else if (i == recv_sock) {
					str_len = read(recv_sock, buf, BUF_SIZE);
					if (str_len == 0) {
						FD_CLR(recv_sock, &reads);
						close(recv_sock);
						printf("closed client: %d \n", i);
					}
					else {
						write(send_sock, buf, str_len);
					}
				}
			}
		}
	}
	close(recv_sock);

	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
