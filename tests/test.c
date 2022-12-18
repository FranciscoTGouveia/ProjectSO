#include "../fs/operations.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main() {
    tfs_init(NULL);
    char* path = "/f1";
    char* txt = "ola ";
    char teste[(strlen(txt) + 1)*3];
    int fd = tfs_open(path,TFS_O_CREAT | TFS_O_APPEND);
    tfs_write(fd, txt,sizeof(txt));
    tfs_close(fd);
    fd = tfs_open(path,TFS_O_APPEND);
    tfs_write(fd, txt,sizeof(txt));
    tfs_close(fd);
    fd = tfs_open(path,TFS_O_APPEND);
    tfs_write(fd, txt,sizeof(txt));
    tfs_close(fd);
    fd = tfs_open(path,0);
    tfs_read(fd,teste,sizeof(txt));
    tfs_close(fd);
    printf("%s\n",teste);
    return 0;
}