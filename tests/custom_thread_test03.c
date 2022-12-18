#include "../fs/operations.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

uint8_t const file_contents[] = "SO PROJECT!";
char const fp[] = "/f1";

void assert_empty_file(char const *path) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(file_contents)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);
}

void *th_run01() {
    assert_empty_file(fp);
    return 0;
}

int main() {
    assert(tfs_init(NULL) != -1);
    int fd = tfs_open(fp, TFS_O_CREAT);
    assert(fd != -1);
    assert(tfs_close(fd) != -1);

    // Creates threads
    pthread_t t1, t2, t3;

    // run1 - Read an empty file
    assert(pthread_create(&t1, NULL, th_run01, NULL) == 0);
    assert(pthread_create(&t2, NULL, th_run01, NULL) == 0);
    assert(pthread_create(&t3, NULL, th_run01, NULL) == 0);

    // Joins threads
    assert(pthread_join(t1, NULL) == 0);
    assert(pthread_join(t2, NULL) == 0);
    assert(pthread_join(t3, NULL) == 0);


    assert(tfs_destroy() != -1);

    printf("Successful test.\n");
    return 0;
}
