/*
Week 12
HW #07: hw07.c
이름: 박석현
학번: 2020110973
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TTL 64
#define NAME_SIZE 30
#define BUF_SIZE 100
#define CONTAINER_SIZE (NAME_SIZE + BUF_SIZE + 3 + 1) // 3 for sprintf, 1 for null

void error_handling(char *message);

int main(int argc, char *argv[]) {
    pid_t pid;
    int time_live = TTL;
    char name[NAME_SIZE];
    char buf[BUF_SIZE];
    char container[CONTAINER_SIZE];

    if (argc != 4) {
        printf("Usage: %s <GroupIP> <PORT> <Name>\n", argv[0]);
        exit(1);
    }
    else {
        strcpy(name, argv[3]);
    }

    pid = fork();

    // 자식 프로세스: Multicast Receiver
    if (pid == 0) {
        int recv_sock;
        int str_len;
        struct sockaddr_in adr;
        struct ip_mreq join_adr;
        int option, optlen;

        recv_sock = socket(PF_INET, SOCK_DGRAM, 0);

        memset(&adr, 0, sizeof(adr));
        adr.sin_family = AF_INET;
        adr.sin_addr.s_addr = htonl(INADDR_ANY);
        adr.sin_port = htons(atoi(argv[2]));

        option = 1;
        optlen = sizeof(option);
        setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &option, optlen);

        if (bind(recv_sock, (struct sockaddr*)&adr, sizeof(adr)) == -1) {
            error_handling("bind() error");
        }

        join_adr.imr_multiaddr.s_addr=inet_addr(argv[1]);
	    join_adr.imr_interface.s_addr=htonl(INADDR_ANY);

        setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));

        while (1) {
            memset(container, 0, CONTAINER_SIZE);

            str_len = recvfrom(recv_sock, container, CONTAINER_SIZE, 0, NULL, 0);
            if (str_len < 0) {
                break;
            }
            else {
                container[str_len] = 0;
                printf("%s", container);
            }
        }
        close(recv_sock);
        return 0;
    }

    // 부모 프로세스: Multicast Sender
    else {
        int send_sock;
        struct sockaddr_in mul_adr;

        send_sock=socket(PF_INET, SOCK_DGRAM, 0);

        memset(&mul_adr, 0, sizeof(mul_adr));
        mul_adr.sin_family=AF_INET;
        mul_adr.sin_addr.s_addr=inet_addr(argv[1]);
        mul_adr.sin_port=htons(atoi(argv[2]));

        setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));

        while (1) {
            memset(buf, 0, BUF_SIZE);
            memset(container, 0, CONTAINER_SIZE);

            fgets(buf, BUF_SIZE, stdin);
            if (strcmp(buf, "q\n") == 0 || strcmp(buf, "Q\n") == 0) {
                printf("SIGKILL: Multicast Receiver terminate!\n");
                printf("Multicast Sender(Parent process) exit!\n");
                kill(pid, SIGKILL);
                close(send_sock);
                break;
            }
            else {
                sprintf(container, "[%s] %s", name, buf);
                container[CONTAINER_SIZE] = 0;

                sendto(send_sock, container, strlen(container), 0, (struct sockaddr*)&mul_adr, sizeof(mul_adr));
            }
        }
    }

    return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
