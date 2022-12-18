#include "../fs/operations.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

uint8_t const file_contents[] = "SO PROJECT!";
char const fp[] = "/f1";

void assert_contents_ok(char const *path) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(file_contents)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == sizeof(buffer));
    assert(memcmp(buffer, file_contents, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);
}

void write_contents(char const *path) {
    int f = tfs_open(path, TFS_O_APPEND);
    assert(f != -1);

    assert(tfs_write(f, file_contents, sizeof(file_contents)) ==
           sizeof(file_contents));

    assert(tfs_close(f) != -1);
}

void *th_run01() {
    write_contents(fp);
    assert_contents_ok(fp);
    return 0;
}

void *th_run02() {
    tfs_unlink(fp);
    return 0;
}

int main() {
    assert(tfs_init(NULL) != -1);
    int fd = tfs_open(fp, TFS_O_CREAT);
    assert(fd != -1);
    assert(tfs_close(fd) != -1);

    // Creates threads
    pthread_t t1, t2;

    // run1 - Write and read a file
    assert(pthread_create(&t1, NULL, th_run01, NULL) == 0);
    // run2 - Unlink the same file
    assert(pthread_create(&t2, NULL, th_run02, NULL) == 0);

    // Joins both of them
    assert(pthread_join(t1, NULL) == 0);
    assert(pthread_join(t2, NULL) == 0);

    assert(tfs_destroy() != -1);

    printf("Successful test.\n");
    return 0;
}