/*
Week 13
HW #08: bus_server.c
이름: 박석현
학번: 2020110973
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX_CLNT 256
#define MAX_SEAT 20

#define OPERATION_SUCCESS 0         // 동작 성공(예약 현황 조회 성공, 예약 성공, 예약 취소 성공)
#define WRONG_SEAT_NUMBER -1        // 잘못된 좌석번호 입력 - 예약 및 예약 취소
#define RESERVATION_FAILED -2       // 예약 실패 - 이미 예약된 좌석인 경우
#define CANCELLATION_FAILED_1 -3     // 예약 취소 실패 - 예약된 좌석이 아닌 경우
#define CANCELLATION_FAILED_2 -4     // 예약 취소 실패 - 예약된 좌석이지만, 예약자가 아닌 경우

#define INQUIRY 1       // 예약 현황 조회
#define RESERVATION 2   // 예약
#define CANCELLATION 3  // 예약 취소
#define QUIT 4          // 프로그램 종료

// Response Packet (Server -> Client)
typedef struct {
    int command;
    int seatno;
    int seats[MAX_SEAT];
    int result;
} RES_PACKET;

// Request Packet(Client -> Server)
typedef struct {
    int command;
    int seatno;
} REQ_PACKET;

void *handle_clnt(void *arg);
void error_handling(char *msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
int bus_seats[20];

int main(int argc, char *argv[]) {
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1) {
		error_handling("bind() error");
	}
	if (listen(serv_sock, 5)==-1) {
		error_handling("listen() error");
	}

	printf("------------------------\n");
	printf(" Bus Reservation System \n");
	printf("------------------------\n");

	while(1) {
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		printf("Connected client IP: %s , clnt_sock=%d\n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);
	}
	for (int i = 0; i < clnt_cnt; i++) {
		pthread_join(clnt_socks[i], NULL);
	}
	pthread_mutex_destroy(&mutx);

	close(serv_sock);
	return 0;
}

void *handle_clnt(void * arg) {
	int clnt_sock=*((int*)arg);
	int str_len;
    REQ_PACKET req_packet;
    RES_PACKET res_packet;

    while (1) {
        memset(&req_packet, 0, sizeof(REQ_PACKET));
        memset(&res_packet, 0, sizeof(RES_PACKET));

        str_len = read(clnt_sock, &req_packet, sizeof(REQ_PACKET));
		if (str_len == 0) {
			break;
		}
		res_packet.command = req_packet.command;
		res_packet.seatno = req_packet.seatno;

        if (res_packet.command == INQUIRY) {
            res_packet.result = OPERATION_SUCCESS;
        }
        else if (res_packet.command == RESERVATION) {
			pthread_mutex_lock(&mutx);
			if (res_packet.seatno < 1 || res_packet.seatno > 20) {
				res_packet.result = WRONG_SEAT_NUMBER;
			}
			else if (bus_seats[res_packet.seatno-1] != 0) {
				res_packet.result = RESERVATION_FAILED;
			}
			else {
				bus_seats[res_packet.seatno-1] = clnt_sock;
				res_packet.result = OPERATION_SUCCESS;
			}
			pthread_mutex_unlock(&mutx);
        }
        else if (res_packet.command == CANCELLATION) {
			pthread_mutex_lock(&mutx);
			if (res_packet.seatno < 1 || res_packet.seatno > 20) {
				res_packet.result = WRONG_SEAT_NUMBER;
			}
			else if (bus_seats[res_packet.seatno-1] == 0) {
				res_packet.result = CANCELLATION_FAILED_1;
			}
			else if (bus_seats[res_packet.seatno-1] != clnt_sock) {
				res_packet.result = CANCELLATION_FAILED_2;
			}
			else {
				bus_seats[res_packet.seatno-1] = 0;
				res_packet.result = OPERATION_SUCCESS;
			}
			pthread_mutex_unlock(&mutx);
        }
        else if (res_packet.command == QUIT) {
			res_packet.result = OPERATION_SUCCESS;
        }
        else {
            printf("Invalid command received: %d\n", res_packet.command);
        }
		memcpy(res_packet.seats, bus_seats, sizeof(bus_seats));
        write(clnt_sock, &res_packet, sizeof(RES_PACKET));
    }
    printf("Client removed: clnt_sock=%d\n", clnt_sock);
	close(clnt_sock);
	return NULL;
}

void error_handling(char * msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
