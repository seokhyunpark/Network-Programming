/*
Week 6
HW #03: hw03_server.c
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
#include <time.h>

#define BOARD_SIZE 3
#define INIT_VALUE 0
#define S_VALUE 1
#define C_VALUE 2

typedef struct {
    int board[BOARD_SIZE][BOARD_SIZE];
} GAMEBOARD;

void error_handling(char *message);
void draw_board(GAMEBOARD *gboard);
void init_board(GAMEBOARD *gboard);
int available_space(GAMEBOARD *gboard);
void choose_space(GAMEBOARD *gboard);

int main(int argc, char *argv[]) {
	srand(time(NULL));

	int serv_sock;
	struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

	// my variables
	int is_empty;
	GAMEBOARD gameboard;

	// 포트번호 입력 여부 확인
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[1]);
        exit(1);
    }

	// start
	printf("Tic-Tac_Toe Server\n");
	init_board(&gameboard);  // 보드 초기화
	draw_board(&gameboard);  // 보드 출력
	
    // socket
	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock == -1) {
		error_handling("socket() error");
    }
	
    // 서버 IP주소, 포트번호 설정
    memset(&serv_addr, 0, sizeof(serv_addr));  // 초기화
    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // IP주소
    serv_addr.sin_port = htons(atoi(argv[1])); // 포트번호
	
    // bind
	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
		error_handling("bind() error");
    }

	while(1) {
		recvfrom(serv_sock, &gameboard, sizeof(gameboard), 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);  // 수신
		draw_board(&gameboard);  // (클라이언트에서) 수신 후 보드 출력

		is_empty = available_space(&gameboard);
		
		// 빈 공간 있음
		if (is_empty == 1) {
			choose_space(&gameboard);  // 위치 설정
			draw_board(&gameboard);  // (서버가) 선택 후 보드 출력
			sendto(serv_sock, &gameboard, sizeof(gameboard), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);  // 송신
		}
		// 빈 공간 없음
		else {
			printf("No available space. Exit this program.\n");
			break;
		}
	}

	printf("Tic Tac Toe Server Close\n");

	close(serv_sock);
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void draw_board(GAMEBOARD *gboard) {
	/* 보드 출력 */
	char value;
	int i, j;

	printf("+-----------+\n");
	for (i = 0; i < BOARD_SIZE; i++) {
		for (j = 0; j < BOARD_SIZE; j++) {
			if (gboard->board[i][j] == INIT_VALUE)
				value = ' ';  // 초기 값 0
			else if (gboard->board[i][j] == S_VALUE)
				value = '0';  // Server 표시 1
			else if (gboard->board[i][j] == C_VALUE)
				value = 'X';  // Client 표시 2
			else
				value = ' ';
			printf("| %c ", value);
		}
		printf("|");
		printf("\n+-----------+\n");
	}
}

void init_board(GAMEBOARD *gboard) {
	/* 보드 초기화 */
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			gboard->board[i][j] = INIT_VALUE;
}

int available_space(GAMEBOARD *gboard) {
	/* 선택 가능한 공간이 있는지 확인 */
	/* return:
		1: 빈 공간이 있는 경우
		0: 빈공간이 없는 경우(full) */
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			if (gboard->board[i][j] == INIT_VALUE)
				return 1;
	return 0;
}

void choose_space(GAMEBOARD *gboard) {
	/* 빈 공간에 위치 설정 */
	int i, j;
	while (1) {
		i = rand() % 3;
		j = rand() % 3;
		// printf("Server try: [%d, %d]\n", i, j);
		if (gboard->board[i][j] == INIT_VALUE) {
			gboard->board[i][j] = S_VALUE;
			break;
		}
	}
	printf("Server choose: [%d, %d]\n", i, j);
}
