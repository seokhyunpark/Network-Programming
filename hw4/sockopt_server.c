/*
Week 9
HW #04: sockopt_server.c
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
#include <netinet/tcp.h>

#define BUF_SIZE 100

typedef struct {
    int level;
    int option;
    int optval; // 요청한 소켓 옵션의 값
    int result; // 요청한 소켓 옵션 결과 (0: 성공, -1: 실패)
} SO_PACKET;

void error_handling(char *message);
void print_blanks();

int main(int argc, char *argv[])
{
	int serv_sock;
    int tcp_sock;
	struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    // variables
    char *option_name_str[10] = {
        "",
        "SO_SNDBUF", // 1
        "SO_RCVBUF", // 2
        "SO_REUSEADDR", // 3
        "SO_KEEPALIVE", // 4
        "SO_BROADCAST", // 5
        "IP_TOS", // 6
        "IP_TTL", // 7
        "TCP_NODELAY", // 8
        "TCP_MAXSEG", // 9
    };
    int option_name[10] = {
        0,
        SO_SNDBUF,
        SO_RCVBUF,
        SO_REUSEADDR,
        SO_KEEPALIVE,
        SO_BROADCAST,
        IP_TOS,
        IP_TTL,
        TCP_NODELAY,
        TCP_MAXSEG,
    };
    int protocol_level[10] = {
        0,
        SOL_SOCKET, // SO_SNDBUF
        SOL_SOCKET, // SO_RCVBUF
        SOL_SOCKET, // SO_REUSEADDR
        SOL_SOCKET, // SO_KEEPALIVE
        SOL_SOCKET, // SO_BROADCAST
        IPPROTO_IP, // IP_TOS
        IPPROTO_IP, // IP_TTL
        IPPROTO_TCP, // TCP_NODELAY
        IPPROTO_TCP // TCP_MAXSEG
    };
    SO_PACKET send_packet;
    SO_PACKET recv_packet;

    tcp_sock = socket(PF_INET, SOCK_STREAM, 0);

	// 포트번호 입력 여부 확인
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[1]);
        exit(1);
    }

    printf("Socket Option Server Start\n");
	
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

	// server: recvfrom -> sendto
	while(1) {
        print_blanks();

		recvfrom(serv_sock, &recv_packet, sizeof(SO_PACKET), 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        printf(">>> Received Socket option: %s\n", option_name_str[recv_packet.option]);

        int result;
        int sock_type;
        int optlen = sizeof(sock_type);
        
        result = getsockopt(tcp_sock, protocol_level[recv_packet.option], option_name[recv_packet.option], (void*)&sock_type, &optlen);
        
        send_packet.level = recv_packet.option;
        send_packet.option = recv_packet.option;
        send_packet.optval = sock_type;
        send_packet.result = result;

		sendto(serv_sock, &send_packet, sizeof(SO_PACKET), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
        printf("<<< Send option: %s: %d, result: %d\n", option_name_str[recv_packet.option], send_packet.optval, send_packet.result);
	}

	close(serv_sock);
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void print_blanks() {
    for (int i = 0; i < 13; i++) {
        printf("\n");
    }
}
