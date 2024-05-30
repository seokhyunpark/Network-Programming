/*
Week 10
HW #05: hw05.c
이름: 박석현
학번: 2020110973
*/

#define _GNU_SOURCE  // sigaction 오류 알림 제거용

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

int i = 0;
int time_out = 0;
int elapsed_time = 0;

void sigint_handler(int sig);
void sigchld_handler(int sig);
void child_sigalarm_handler(int sig);
void parent_sigalarm_handler(int sig);

int main() {
    pid_t pid;

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    pid = fork();

    // 자식 프로세스
    if (pid == 0) {
        printf("Child process created.\n");

        act.sa_handler = child_sigalarm_handler;
        sigaction(SIGALRM, &act, 0);

        alarm(5);

        while (i < 5) {
            sleep(1);
        }
        return 5;
    }
    // 부모 프로세스
    else {
        printf("Parent process created.\n");

        act.sa_handler = parent_sigalarm_handler;
        sigaction(SIGALRM, &act, 0);

        act.sa_handler = sigchld_handler;
        sigaction(SIGCHLD, &act, 0);

        act.sa_handler = sigint_handler;
        sigaction(SIGINT, &act, 0);

        alarm(2);

        while (1) {
            sleep(1);
        }
    }
    
    return 0;
}

void sigint_handler(int sig) {
    if (sig == SIGINT) {
        char c;
        printf("SIGINT: Do you want to eixt (y or Y to exit)? ");
        scanf("%c", &c);
        if (c == 'y' || c == 'Y') {
            exit(0);
        }
    }
}

void sigchld_handler(int sig) {
    int status;
    pid_t id = waitpid(-1, &status, WNOHANG);
    if (WIFEXITED(status)) {
        printf("Child: %d, sent: %d\n", id, WEXITSTATUS(status));
    }
}

void child_sigalarm_handler(int sig) {
    if (sig == SIGALRM) {
        time_out = 5;
        elapsed_time += time_out;
        printf("[Child]  time out: %d, elapsed time: %3d seconds(%d)\n", time_out, elapsed_time, ++i);

        alarm(5);
    }
}

void parent_sigalarm_handler(int sig) {
    if (sig == SIGALRM) {
        time_out = 2;
        elapsed_time += time_out;
        printf("<Parent> time out: %d, elapsed time: %3d seconds\n", time_out, elapsed_time);

        alarm(2);
    }
}
