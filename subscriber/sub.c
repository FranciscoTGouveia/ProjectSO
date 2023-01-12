#include "../utils/logging.h"
#include "../utils/pipeflow.h"
#include "../utils/writer.h"
#include "../utils/reader.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

int fd_fifo;
char* pipe_name;

void getCTRLC(int s) {
    (void) s;
    ssize_t t = write(1,"ESTOU NO SGINAL",strlen("ESTOU NO SGINAL"));
    (void) t;
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
    char buffer[MAX_LINE] = "";
    writer(&newrequest, newrequest.code, buffer);
    printf("Mensagem a mandar no pipe %s tamanho %ld \n", buffer, strlen(buffer));
    int fd = open(argv[1], O_WRONLY);
    ssize_t value = write(fd, buffer, strlen(buffer));
    printf("Tamanho que foi escrito %ld \n", value);
    value++;
    close(fd);
    if ((fd_fifo = mkfifo(argv[2], 0777)) < 0) {
        exit(1);
    }
    signal(SIGINT, getCTRLC);
    while (1) {
        fd = open(argv[2], O_RDONLY);
        if (fd < 0) {exit(1);}
        char message[MAX_LINE];
        value = read(fd, message, sizeof(message));
        printf("mensagem do pipe %s\n", message);
        value++;
        strtok(message, "|");
        messages_pipe* newmesage = reader(10);
        fprintf(stdout, "%s\n", newmesage->message);
        free(newmesage);
        close(fd);
    }
    close(fd_fifo);
    unlink(argv[2]);
    fprintf(stderr, "usage: sub <register_pipe_name> <box_name>\n");
    WARN("unimplemented"); // TODO: implement
    return -1;
}
