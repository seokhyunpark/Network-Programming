/*
Week 13
HW #08: bus_client.c
이름: 박석현
학번: 2020110973
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>

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
	
	
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
void print_bus_status(int bus_status[]);

static sem_t sem_recv;
static sem_t sem_send;


int main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t send_thread;
    pthread_t recv_thread;
    void * thread_return;

    // 순서 설정: 송신 -> 수신
    sem_init(&sem_recv, 0, 0);
    sem_init(&sem_send, 0, 1);

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
    }

	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) {
        error_handling("connect() error");
    }
	pthread_create(&send_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&recv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(send_thread, &thread_return);
	pthread_join(recv_thread, &thread_return);
	close(sock);
	return 0;
}

void *send_msg(void * arg)
{
    int sock=*((int*)arg);
    REQ_PACKET req_packet;

    while (1) {
        memset(&req_packet, 0, sizeof(REQ_PACKET));

        sem_wait(&sem_send);
        printf("1: inquiry, 2: reservation, 3: cancellation, 4: quit: ");
        scanf("%d", &req_packet.command);

        if (req_packet.command == INQUIRY) {
            write(sock, &req_packet, sizeof(REQ_PACKET));
            sem_post(&sem_recv);
        }
        else if (req_packet.command == RESERVATION) {
            printf("Input seat number: ");
            scanf("%d", &req_packet.seatno);
            write(sock, &req_packet, sizeof(REQ_PACKET));
            sem_post(&sem_recv);
        }
        else if (req_packet.command == CANCELLATION) {
            printf("Input seat number for cancellation: ");
            scanf("%d", &req_packet.seatno);
            write(sock, &req_packet, sizeof(REQ_PACKET));
            sem_post(&sem_recv);
        }
        else if (req_packet.command == QUIT) {
            write(sock, &req_packet, sizeof(REQ_PACKET));
            sem_post(&sem_recv);
            break;
        }
        else {
            printf("Invalid input. Please try again.\n");
            write(sock, &req_packet, sizeof(REQ_PACKET));
            sem_post(&sem_recv);
        }
    }

    return NULL;
}
	
void *recv_msg(void * arg)
{
	int sock=*((int*)arg);
    RES_PACKET res_packet;

	while(1) {
        memset(&res_packet, 0, sizeof(RES_PACKET));

        sem_wait(&sem_recv);
        read(sock, &res_packet, sizeof(RES_PACKET));
        if (res_packet.command == QUIT) {
            printf("QUIT\n");
            sem_post(&sem_send);
            break;
        }

        if (res_packet.command >= 1 && res_packet.command <= 4) {
            print_bus_status(res_packet.seats);

            if (res_packet.result == OPERATION_SUCCESS) {
                printf("Operation sucess.\n");
            }
            else if (res_packet.result == WRONG_SEAT_NUMBER) {
                printf("Wrong seat number\n");
            }
            else if (res_packet.result == RESERVATION_FAILED) {
                printf("Reservation failed. (The seat was already reserved)\n");
            }
            else if (res_packet.result == CANCELLATION_FAILED_1) {
                printf("Cancellation failed. (The seat was not reserved)\n");
            }
            else if (res_packet.result == CANCELLATION_FAILED_2) {
                printf("Cancellation failed. (The seat was reserved by another person)\n");
            }
        }
        sem_post(&sem_send);
	}

	return NULL;
}

void print_bus_status(int bus_status[]) {
    int i = 0;

    printf("---------------------------------------------------\n");
    printf("             Bus Reservation Status                \n");
    printf("---------------------------------------------------\n");

    for (i = 0; i < 10; i++) {
        printf("|");
        printf("%3d ", i+1);
    }
    printf("|\n");
    printf("---------------------------------------------------\n");

    for (i = 0; i < 10; i++) {
        printf("|");
        printf("%3d ", bus_status[i]);
    }
    printf("|\n");
    printf("---------------------------------------------------\n");

    for (i = 10; i < 20; i++) {
        printf("|");
        printf("%3d ", i+1);
    }
    printf("|\n");
    printf("---------------------------------------------------\n");

    for (i = 10; i < 20; i++) {
        printf("|");
        printf("%3d ", bus_status[i]);
    }
    printf("|\n");
    printf("---------------------------------------------------\n");
}
	
void error_handling(char *msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
