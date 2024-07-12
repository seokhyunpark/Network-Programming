/*
기말고사
land_server.c
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

int occupied = 0;
int board[ROW][COL] = {0};

int clnt_cnt=0;
int clnt_socks[256];
pthread_mutex_t mutx;

void *handle_clnt(void * arg);
void draw_board(RES_PACKET *res_packet);
void error_handling(char * msg);

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

	while (1) {
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		printf("Connected client IP: %s\n", inet_ntoa(clnt_adr.sin_addr));
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

	res_packet.cmd = GAME_READY;
	res_packet.result = SUCCESS;
	memset(&board, 0, sizeof(board));
	memcpy(res_packet.board, board, sizeof(board));
	write(clnt_sock, &res_packet, sizeof(res_packet)); // GAME_READY 전송
	printf("[Tx] cmd: %d, client_id: %d, result: %d\n", res_packet.cmd, clnt_sock, res_packet.result);

    while (1) {
		pthread_mutex_lock(&mutx);
		memset(&res_packet, 0, sizeof(res_packet));
		memset(&req_packet, 0, sizeof(req_packet));
		
		read(clnt_sock, &req_packet, sizeof(req_packet)); // GAME_REQUEST 수신

		if (req_packet.cmd == GAME_REQUEST) {
			int row = req_packet.row;
			int col = req_packet.col;
			printf("[Rx] client: %d, cmd: %d, index:(%d, %d)\n", clnt_sock, req_packet.cmd, row, col);

			if (board[row][col] == 0) {
				res_packet.result = SUCCESS;
				board[row][col] = clnt_sock;
				occupied++;
			}
			else {
				res_packet.result = FAIL;
				printf("[Client %d] already chose.(%d, %d)\n", board[row][col], row, col);
			}

			res_packet.cmd = GAME_RESPONSE;
			memcpy(res_packet.board, board, sizeof(board));
			write(clnt_sock, &res_packet, sizeof(res_packet));
			printf("[Tx] cmd: %d, client_id: %d, result: %d\n", res_packet.cmd, clnt_sock, res_packet.result);

			draw_board(&res_packet);

			if (occupied == ROW * COL) {
				res_packet.cmd = GAME_END;
				write(clnt_sock, &res_packet, sizeof(res_packet));
				close(clnt_sock);
				printf("Close client_sock :%d, client_cnt: %d\n", clnt_sock, clnt_cnt);
				pthread_mutex_unlock(&mutx);
				return NULL;
			}
		}
		pthread_mutex_unlock(&mutx);
	}
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
	printf("Occupied: %d\n", occupied);
}

void error_handling(char * msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void calculate_board(RES_PACKET *res_packet, int id) {
	int bigger_board[ROW+2][COL+2] = {0};

	for (int i = 0; i < ROW+2; i++) {
		for (int j = 0; j < COL+2; j++) {
			
		}
	}
}
