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

int fd_fifo;
char* pipe_name;
int counter = 0;

void getCTRLC(int s) {
    (void) s;
    char buffer[100];
    int value = snprintf(buffer, sizeof(buffer) - 1, "%d", counter);
    if (write(STDOUT_FILENO,"ESTOU NO SIGNAL\n",strlen("ESTOU NO SGINAL\n")) < 0) {
        exit(1);
    }
    if (write(STDOUT_FILENO, buffer, (size_t)value) < 0) {
        exit(1);
    }
    close(fd_fifo);
    signal(SIGINT, getCTRLC);
    unlink(pipe_name);
    exit(0);
}

void ignore_sigpipe(int s) {
    (void) s;

}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    pipe_name = argv[2];
    request newrequest;
    newrequest.code = 2;
    strcpy(newrequest.pipe_name, argv[2]);
    char box_name_slash[MAX_BOX_NAME];
    memset(box_name_slash, 0, MAX_BOX_NAME);
    strcpy(box_name_slash, "/");
    strcat(box_name_slash, argv[3]);
    strcpy(newrequest.box_name, box_name_slash);
    char buffer[MAX_LINE];
    writer_stc(&newrequest, newrequest.code, buffer);
    printf("Mensagem a mandar no pipe %s tamanho %ld \n", buffer, strlen(buffer));
    int fd = open(argv[1], O_WRONLY);
    ssize_t value = write(fd, buffer, sizeof(buffer));
    printf("Tamanho que foi escrito %ld \n", value);
    value++;
    close(fd);
    if (mkfifo(argv[2], 0777) < 0) {
        exit(1);
    }
    signal(SIGINT, getCTRLC);
    fd_fifo = open(argv[2], O_RDONLY);
    if (fd_fifo < 0) {exit(1);}
    while (1) {
        char message[MAX_LINE];
        value = read(fd_fifo, message, sizeof(message));
        if (value == -1) {
            break;
        }
        counter++;
        value++;
        //strtok(message, "|");
        messages_pipe* newmesage = reader_stc(message);
        fprintf(stdout, "%s\n", newmesage->message);
        free(newmesage);
    }
    close(fd_fifo);
    unlink(argv[2]);
    return -1;
}
