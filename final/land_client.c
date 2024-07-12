/*
기말고사
land_client.c
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
#include <semaphore.h>

#define ROW 7
#define COL 7

// cmd field value
#define GAME_READY 0 // Server -> Client
#define GAME_REQUEST 1 // Client -> Server
#define GAME_RESPONSE 2 // Server -> Client
#define GAME_END 3 // Server -> Client

// result 값
#define FAIL 0 // 해당 인덱스에 다른 클라이언트가 선택한 경우
#define SUCCESS 1 // 해당 인덱스를 선택하는 데 성공

// Client -> Server (GAME_REQUEST 메시지)
typedef struct {
	int cmd; // cmd field value
	int row; // 랜덤하게 선택한 row 값
	int col; // 랜덤하게 선택한 col 값
} REQ_PACKET;

// Server -> Client (GAME_READY, GAME_RESPONSE, GAME_END 메시지)
typedef struct {
	int cmd;
	int board[ROW][COL];
	int result;
} RES_PACKET;

void * send_msg(void * arg);
void * recv_msg(void * arg);
void draw_board(RES_PACKET *res_packet);
void error_handling(char * msg);

static sem_t sem_recv;
static sem_t sem_send;

int main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t send_thread;
    pthread_t recv_thread;
    void * thread_return;

	// 순서 설정: recv -> send
    sem_init(&sem_recv, 0, 1);
    sem_init(&sem_send, 0, 0);
    
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


    return 0;
}

void error_handling(char * msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void *send_msg(void *arg) {
    int sock=*((int*)arg);
	RES_PACKET res_packet;
	REQ_PACKET req_packet;

	while (1) {
		sem_wait(&sem_send);
		memset(&res_packet, 0, sizeof(res_packet));
		memset(&req_packet, 0, sizeof(req_packet));

		req_packet.cmd = GAME_REQUEST;
		req_packet.row = rand() % ROW;
		req_packet.col = rand() % COL;
		write(sock, &req_packet, sizeof(req_packet));
		printf("[Tx] cmd: %d, index(%d, %d)\n", req_packet.cmd, req_packet.row, req_packet.col);

		sleep(1);
		sem_post(&sem_recv);
	}
	return NULL;
}

void *recv_msg(void *arg) {
	int sock=*((int*)arg);
	RES_PACKET res_packet;
	REQ_PACKET req_packet;

	while (1) {
		sem_wait(&sem_recv);
		memset(&res_packet, 0, sizeof(res_packet));
		memset(&req_packet, 0, sizeof(req_packet));

		read(sock, &res_packet, sizeof(res_packet));
		printf("[Rx] cmd: %d, result: %d\n", res_packet.cmd, res_packet.result);

		if (res_packet.cmd == GAME_READY) {
			req_packet.cmd = res_packet.cmd;
		}
		else if (res_packet.cmd == GAME_RESPONSE) {
			draw_board(&res_packet);
		}
		else if (res_packet.cmd == GAME_END) {
			printf("GAME_END. Game is over!\n");
			exit(0);
		}
		sem_post(&sem_send);
	}
	return NULL;
}

void draw_board(RES_PACKET *res_packet) {
	char value;
	int i, j;

	printf("+----------------------------------+\n");
	for (i = 0; i < ROW; i++) {
		for (j = 0; j < COL; j++) {
			printf("| %2d ", res_packet->board[i][j]);
		}
		printf("|");
		printf("\n+----------------------------------+\n");
	}
}
