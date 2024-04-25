/*
중간고사
udp_server.c
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

char board_data[BOARD_SIZE][BOARD_SIZE];
RES_PACKET server;

void error_handling(char *message);
void init_board(RES_PACKET *gboard);
void create_server_board();
void draw_board(RES_PACKET *gboard);
void compare_board(REQ_PACKET *req_packet, RES_PACKET *res_packet);
int done_check();

int main(int argc, char *argv[]) {
	int serv_sock;
	struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    // variables
    int is_done;
    REQ_PACKET req_packet;
    RES_PACKET res_packet;

	// 포트번호 입력 여부 확인
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[1]);
        exit(1);
    }

    // start
    printf("---------------------------------\n");
    printf("   Finding Alphabet Game Server  \n");
    printf("---------------------------------\n");
    create_server_board();  // 서버 보드 생성 (랜덤 알파벳)
    draw_board(&server);
    init_board(&res_packet);  // res_packet 초기화
    draw_board(&res_packet);
    printf("\n");
	
    // socket
	serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock==-1) {
		error_handling("socket() error");
    }
	
    // 서버 IP주소, 포트번호 설정
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
    // bind
	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) {
		error_handling("bind() error");
    }

	while(1) {
		recvfrom(serv_sock, &req_packet, sizeof(req_packet), 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);  // 수신
        // cmd가 GAME_REQ인 경우
        if (req_packet.cmd == GAME_REQ) {
            printf("[Server] Rx cmd=%d, ch=%c\n", req_packet.cmd, req_packet.ch);

            // 모든 알파벳을 맞춘 경우
            is_done = done_check();
            if (is_done == 1) {
                printf("No empty space. Exit this program.\n");
                res_packet.cmd = GAME_END;
                res_packet.result = 0;
                sendto(serv_sock, &res_packet, sizeof(res_packet), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);  // 송신
                printf("[Server] Tx cmd=%d, result=%d\n", res_packet.cmd, res_packet.result);
                break;
            }
            // 아직 맞출 알파벳이 남아 있는 경우
            else {
                compare_board(&req_packet, &res_packet);

                // 클라이언트로 전송(알파벳, 개수)
                res_packet.cmd = GAME_RES;
                sendto(serv_sock, &res_packet, sizeof(res_packet), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);  // 송신
                printf("[Server] Tx cmd=%d, result=%d\n", res_packet.cmd, res_packet.result);
                
                // 보드 출력
                draw_board(&server);
                draw_board(&res_packet);
                printf("\n");
            }
            // 1초 딜레이
            sleep(1);
        }
        // 예외 처리
        else {
            printf("Invalid cmd: %d\n", req_packet.cmd);
        }
	}

    printf("Exit Server Program\n");

	close(serv_sock);
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

void create_server_board() {
	/* 서버 보드 랜덤 생성 */
    srand(time(NULL));
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			server.board[i][j] = 'A' + (rand() % 26);
}

void draw_board(RES_PACKET *res_packet) {
	/* 보드 출력 */
	char value;
	int i, j;

	printf("+-------------------+\n");
	for (i = 0; i < BOARD_SIZE; i++) {
		for (j = 0; j < BOARD_SIZE; j++) {
			if (res_packet->board[i][j] == 0)
				value = ' ';
			else
				value = res_packet->board[i][j];
			printf("| %c ", value);
		}
		printf("|");
		printf("\n+-------------------+\n");
	}
}

void compare_board(REQ_PACKET *req_packet, RES_PACKET *res_packet) {
    /* 랜덤 생성 배열과 클라이언트가 맞춘 알파벳 비교*/
    res_packet->result = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (server.board[i][j] == req_packet->ch) {
                res_packet->board[i][j] = req_packet->ch;
                res_packet->result++;
                board_data[i][j] = 1;
            }
        }
    }
}

int done_check() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board_data[i][j] == 0) {
                return 0;
            }
        }
    }
    return 1;
}
