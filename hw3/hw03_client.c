/*
Week 6
HW #03: hw03_client.c
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

int main(int argc, char *argv[])
{
	int sock;
	
	struct sockaddr_in serv_addr;
    struct sockaddr_in from_addr;
    socklen_t from_addr_size=sizeof(from_addr);

	// my variables
	int i, j;
	int is_empty;
	GAMEBOARD gameboard;

    // IP주소, 포트번호 입력 여부 확인
	if(argc != 3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	// start
	printf("Tic-Tac-Toe Client\n");
	init_board(&gameboard);  // 보드 초기화
	draw_board(&gameboard);  // 보드 출력

    // socket
	sock = socket(PF_INET, SOCK_DGRAM, 0);   
	if(sock == -1) {
		error_handling("socket() error");
    }
	
    // 서버 IP주소, 포트번호 설정
    memset(&serv_addr, 0, sizeof(serv_addr));  // 초기화
    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // IP주소
    serv_addr.sin_port = htons(atoi(argv[2])); // 포트번호

	while(1) {
		printf("Input row, column: ");
		scanf("%d %d", &i, &j);

		// 배열 내 인덱스 (0 <= [i, j] <= 3)
		if (i >= 0 && i <= BOARD_SIZE && j >= 0 && j <= BOARD_SIZE) {
			// 빈 인덱스
			if (gameboard.board[i][j] == INIT_VALUE) {
				gameboard.board[i][j] = C_VALUE;
				draw_board(&gameboard);  // (클라이언트가) 선택 후 보드 출력
				sendto(sock, &gameboard, sizeof(gameboard), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));  // 송신

				is_empty = available_space(&gameboard);

				// 빈 공간 있음
				if (is_empty == 1) {
					recvfrom(sock, &gameboard, sizeof(gameboard), 0, (struct sockaddr*)&from_addr, &from_addr_size);  // 수신
					draw_board(&gameboard);  // (서버에서) 수신 후 보드 출력
				}
				// 빈 공간 없음
				else {
					printf("No available space. Exit Client\n");
					break;
				}
			}
			// 이미 사용중인 인덱스
			else {
				printf("Already occupied index\n");
			}
		}
		// 배열 밖 인덱스
		else {
			continue;
		}
	}

	printf("Tic Tac Toe Client Close\n");

	close(sock);
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
