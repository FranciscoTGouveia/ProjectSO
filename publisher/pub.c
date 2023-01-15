#include "../utils/logging.h"
#include "../utils/pipeflow.h"
#include "../utils/writer_stc.h"
#include "../utils/safety_mechanisms.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#define MAX_MESSAGE 1024


void ignore_signal(int s) {
    (void) s;
    signal(SIGPIPE, ignore_signal);
}


int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: pub <register_pipe_name> <box_name>\n");
    }
    request newrequest;
    newrequest.code = 1;
    strcpy(newrequest.pipe_name, argv[2]);
    char box_name_slash[MAX_BOX_NAME];
    memset(box_name_slash, 0, MAX_BOX_NAME);
    strcpy(box_name_slash, "/");
    strcat(box_name_slash, argv[3]);
    strcpy(newrequest.box_name, box_name_slash);
    char buffer[MAX_LINE];
    writer_stc(&newrequest, newrequest.code, buffer);
    my_mkfifo(argv[2], 0777);
    int fd = my_open(argv[1], O_WRONLY);
    if (write(fd, buffer, sizeof(buffer)) < 0) {
        my_close(fd);
        exit(1);
    } 
    my_close(fd);
    signal(SIGPIPE, ignore_signal);
    fd = my_open(argv[2], O_WRONLY);
    while (1) {
        char message[MAX_MESSAGE];
        if (fgets(message, sizeof(message), stdin) == NULL) {
            // Will catch an EOF generated by ctrl+d
            my_close(fd);
            my_unlink(argv[2]);
            return 0;
        }
        if (message[strlen(message)-1] == '\n') {
            message[strlen(message)-1] = '\0';
        }
        char server_request[MAX_LINE];
        messages_pipe new_message;
        new_message.code = 9;
        strcpy(new_message.message, message);
        writer_stc(&new_message, new_message.code, server_request);
        if (write(fd, server_request, sizeof(server_request)) < 0) { // Write the request to mbroker
            if (errno == EPIPE) {
                break;
            }
        }
    }
    my_close(fd);
    my_unlink(argv[2]);
    return -1;
}
