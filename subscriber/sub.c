#include "../utils/logging.h"
#include "../utils/pipeflow.h"
#include "../utils/reader_stc.h"
#include "../utils/writer_stc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

int fd;
char* pipe_name;
int counter = 0;

void getCTRLC(int s) {
    (void) s;
    char buffer[100];
    // Why is this here ???
    int value = snprintf(buffer, sizeof(buffer) - 1, "%d", counter);
    if (write(STDOUT_FILENO, buffer, (size_t)value) < 0) {
        exit(1);
    }
    close(fd);
    // Why is this here ???
    signal(SIGINT, getCTRLC);
    unlink(pipe_name);
    exit(0);
}

// Não se devia ligar o signal SIGPIPE a isto ???
void ignore_sigpipe(int s) {
    (void) s;
}

int main(int argc, char **argv) {
    (void)argc;
    pipe_name = argv[2];
    request newrequest;
    newrequest.code = 2;
    strcpy(newrequest.pipe_name, pipe_name);
    char box_name_slash[MAX_BOX_NAME];
    memset(box_name_slash, 0, MAX_BOX_NAME);
    strcpy(box_name_slash, "/");
    strcat(box_name_slash, argv[3]);
    strcpy(newrequest.box_name, box_name_slash);
    char server_request[MAX_LINE];
    writer_stc(&newrequest, newrequest.code, server_request);
    if ((fd = open(argv[1], O_WRONLY)) < 0) exit(1);
    if (write(fd, server_request, sizeof(server_request)) < 0) exit(1); 
    close(fd);
    if (mkfifo(argv[2], 0777) < 0) exit(1);
    signal(SIGINT, getCTRLC);
    if ((fd = open(argv[2], O_RDONLY)) < 0) exit(1);
    while (1) {
        char message[MAX_LINE];
        if (read(fd, message, sizeof(message)) == 0) break;
        counter++;
        messages_pipe* newmesage = reader_stc(message);
        fprintf(stdout, "%s\n", newmesage->message);
        free(newmesage);
    }
    close(fd);
    unlink(pipe_name);
    return -1;
}
