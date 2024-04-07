/*
Week 3
LAB #01: mymove.c
이름: 박석현
학번: 2020110973
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 10

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("[Error] mymove Usage: %s src_file dest_file\n", argv[0]);
        exit(1);
    }

    int src, dest;
    int size, bytes;
    int r;
    char buf[BUF_SIZE];

    src = open(argv[1], O_RDONLY);
	if(src == -1) {
		printf("open() src error!\n");
        exit(1);
    }
    
    dest = open(argv[2], O_CREAT|O_WRONLY|O_TRUNC, 0644);
    if(dest == -1) {
        printf("open() dest error!\n");
        exit(1);
    }
    
    while (r = read(src, buf, sizeof(buf)) != 0) {
        size = write(dest, buf, r); // not strlen(buf)
        bytes += size;
    }

    close(src);
    close(dest);

    printf("move from %s to %s (bytes: %d) finished.\n", argv[1], argv[2], bytes);

    remove(argv[1]);
    return 0;
}
