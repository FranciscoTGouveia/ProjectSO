#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void* my_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        printf("Falha na alocação de memória.\n");
        // codigo para limpar todas as merdas
        exit(1);
    }
    return ptr;
}

int my_open(char* pathname, int mode) {
    int fd = open(pathname, mode);
    if (fd < 0) exit(1);
    return fd;
}

void my_close(int fd) {
    if (close(fd) == -1) exit(1);
}

void my_unlink(char* pathname) {
    if (unlink(pathname) == -1) exit(1);
}

void my_mkfifo(char* pathname, mode_t mode) {
    if (mkfifo(pathname, mode) < 0) exit(1);
}

void my_write(int fd, char* buffer, size_t count) {
    if (write(fd, buffer, count) < 0) exit(1);
}