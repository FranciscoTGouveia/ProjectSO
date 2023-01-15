#include "../utils/logging.h"
#include "../utils/pipeflow.h"
#include "../utils/writer_stc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#define MAX_MESSAGE 1024

void ignore_signal(int s) {
    if (write(1,"ESTOU NO SIGNAL DO PUB\n", strlen("ESTOU NO SIGNAL DO PUB\n")) < 0) {

    }
    (void) s;

}


int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    request newrequest;
    newrequest.code = 1;
    strcpy(newrequest.pipe_name, argv[2]);
    char box_name_slash[MAX_BOX_NAME];
    memset(box_name_slash, 0, MAX_BOX_NAME);
    strcpy(box_name_slash, "/");
    strcat(box_name_slash, argv[3]);
    strcpy(newrequest.box_name,box_name_slash);
    char buffer[MAX_LINE];
    writer_stc(&newrequest, newrequest.code, buffer);
    int fd_fifo;
    if ((fd_fifo = mkfifo(argv[2], 0777)) < 0) {
        exit(1);
    }
    int fd = open(argv[1], O_WRONLY);
    if (fd < 0) {return -1;}
    ssize_t value = write(fd, buffer, sizeof(buffer));
    value++;
    close(fd);
    signal(SIGPIPE, ignore_signal);
    fd = open(argv[2], O_WRONLY);
    if (fd < 0) {exit(1);}
    while (1) {
        char message[MAX_MESSAGE];
        if (fgets(message, sizeof(message), stdin) == NULL) {
            close(fd);
            unlink(argv[2]);
            return 0;
        }
        printf("as mensagens dentro do pub %s \n", message);
        if (message[strlen(message) -1] == '\n') {
            message[strlen(message) - 1] = '\0';
        }
        char buffer1[MAX_LINE];
        messages_pipe newmessage;
        newmessage.code = 9;
        strcpy(newmessage.message, message);
        writer_stc(&newmessage, newmessage.code, buffer1);
        value = write(fd, buffer1, sizeof(buffer1));
        if (errno == EPIPE) {
            break;
        }
        value++;
    }
    close(fd);
    unlink(argv[2]);
    return -1;
}
