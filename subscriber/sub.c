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
    int value = snprintf(buffer, sizeof(buffer), "%d", counter);
    buffer[strlen(buffer) - 1] = '\n';
    if (write(STDOUT_FILENO,"ESTOU NO SGINAL\n",strlen("ESTOU NO SGINAL\n")) < 0) {
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


int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    pipe_name = argv[2];
    request newrequest;
    newrequest.code = 2;
    strcpy(newrequest.pipe_name, argv[2]);
    strcpy(newrequest.box_name, argv[3]);
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
    if (fd < 0) {exit(1);}
    while (1) {
        char message[MAX_LINE];
        value = read(fd, message, sizeof(message));
        counter++;
        value++;
        //strtok(message, "|");
        messages_pipe* newmesage = reader_stc(message);
        fprintf(stdout, "%s\n", newmesage->message);
        free(newmesage);
    }
    close(fd_fifo);
    unlink(argv[2]);
    fprintf(stderr, "usage: sub <register_pipe_name> <box_name>\n");
    WARN("unimplemented"); // TODO: implement
    return -1;
}
