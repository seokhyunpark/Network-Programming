/*
중간고사
udp_client.c
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

#define BOARD_SIZE 5
// cmd type
#define GAME_REQ 0
#define GAME_RES 1
#define GAME_END 2
// Request Packet: Client -> Server
typedef struct {
    int cmd; // GAME_REQ
    char ch; // 알파벳 대문자 하나 전송
} REQ_PACKET;

// Response Packet: Server -> Client
typedef struct {
    int cmd; // GAME_RES, GAME_END
    char board[BOARD_SIZE][BOARD_SIZE]; // 맞춘 알파벳만 저장 후 클라이언트로 전송
    int result; // 맞춘 알파벳의 개수 전달
} RES_PACKET;

void error_handling(char *message);
void init_board(RES_PACKET *gboard);
void draw_board(RES_PACKET *res_packet);

int main(int argc, char *argv[]) {
    srand(time(NULL));

	int sock;
	struct sockaddr_in serv_addr;
    struct sockaddr_in from_addr;
    socklen_t from_addr_size=sizeof(from_addr);

    // variables
    REQ_PACKET req_packet;
    RES_PACKET res_packet;

    // start
    printf("---------------------------------\n");
    printf("   Finding Alphabet Game Server  \n");
    printf("---------------------------------\n");
    // init_board(&packet);

    // IP주소, 포트번호 입력 여부 확인
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

    // socket
	sock = socket(PF_INET, SOCK_DGRAM, 0);   
	if(sock==-1) {
		error_handling("socket() error");
    }
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

	while(1) {
        // 랜덤 알파벳 생성 후 서버로 전송
        req_packet.cmd = GAME_REQ;
        req_packet.ch = 'A' + (rand() % 26);

		sendto(sock, &req_packet, sizeof(req_packet), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        printf("[Client] Tx cmd=%d, ch=%c\n", req_packet.cmd, req_packet.ch);

        // 서버로부터 res_packet 수신
		recvfrom(sock, &res_packet, sizeof(res_packet), 0, (struct sockaddr*)&from_addr, &from_addr_size);        
        
        // cmd가 GAME_RES인 경우
        if (res_packet.cmd == GAME_RES) {
            printf("[Client] Rx cmd=%d, result=%d\n", res_packet.cmd, res_packet.result);
            draw_board(&res_packet);
            printf("\n");
        }
        // cmd가 GAME_END인 경우
        else if (res_packet.cmd == GAME_END) {
            printf("[Client] Rx cmd=%d, result=%d\n", res_packet.cmd, res_packet.result);
            printf("No empty space. Exit this program\n");
            break;
        }
        // 예외 처리
        else {
            printf("Invalid cmd: %d\n", res_packet.cmd);
        }
    }

    printf("Exit Client Program\n");

	close(sock);
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void init_board(RES_PACKET *gboard) {
	/* 보드 초기화 */
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			gboard->board[i][j] = 0;
}

void draw_board(RES_PACKET *res_packet) {
	/* 보드 출력 */
	char value;
	int i, j;

	printf("+-------------------+\n");
	for (i = 0; i < BOARD_SIZE; i++) {
		for (j = 0; j < BOARD_SIZE; j++) {
			if (res_packet->board[i][j] == 0)
				value = ' ';  // 초기 값 0
			else
				value = res_packet->board[i][j];
			printf("| %c ", value);
		}
		printf("|");
		printf("\n+-------------------+\n");
	}
}
