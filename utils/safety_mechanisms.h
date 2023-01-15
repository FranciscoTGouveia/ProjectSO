#ifndef __UTILS_SERVER_STRCUTURES__
#define __UTILS_SERVER_STRCUTURES__

#include <sys/types.h>

void* my_malloc(size_t size);
int my_open(char* pathname, int mode);
void my_close(int fd);
void my_unlink(char* pathname);
void my_mkfifo(char* pathname, mode_t mode);
void my_write(int fd, char* buffer, size_t count);

#endif