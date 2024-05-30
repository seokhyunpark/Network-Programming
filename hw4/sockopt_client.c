/*
Week 9
HW #04: sockopt_client.c
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
void print_option_names(char *arr[]);

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
    struct sockaddr_in from_addr;
    socklen_t from_addr_size=sizeof(from_addr);

    // variables
    char *option_name_str[11] = {
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
        "Quit" // 10
    };
    SO_PACKET send_packet;
    SO_PACKET recv_packet;

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

	// client: sendto -> recvfrom
	while(1) {
        print_option_names(option_name_str);

        printf("Input option number: ");
        scanf("%d", &(send_packet.option));

        while (send_packet.option < 1 || send_packet.option > 10) {
            printf("Wrong number. type again!\n");
            printf("Input option number: ");
            scanf("%d", &(send_packet.option));
        }

        if (send_packet.option == 10) {
            printf("Client quit\n");
            break;
        }
        else if (send_packet.option >= 1 && send_packet.option <= 9) {
            sendto(sock, &send_packet, sizeof(SO_PACKET), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            recvfrom(sock, &recv_packet, sizeof(SO_PACKET), 0, (struct sockaddr*)&from_addr, &from_addr_size);
            printf(">>> Server result: %s: value: %d, result: %d\n", option_name_str[recv_packet.option], recv_packet.optval, recv_packet.result);
        }
        else {
            printf("Unexpected input.\n");
        }
		printf("\n\n");
	}	
	close(sock);
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void print_option_names(char *arr[]) {
    printf("---------------------------\n");
        for (int i = 1; i <= 10; i++) {
            printf("%d: %s\n", i, arr[i]);
        }
    printf("---------------------------\n");
}
