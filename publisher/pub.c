#include "../utils/logging.h"
#include "../utils/pipeflow.h"
#include "../utils/writer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define MAX_MESSAGE 1024

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    request newrequest;
    newrequest.code = 1;
    strcpy(newrequest.pipe_name, argv[2]);
    strcpy(newrequest.box_name, argv[3]);
    char buffer[MAX_LINE] = "";
    writer(&newrequest, newrequest.code, buffer);
    if (mkfifo(argv[2], 0777) < 0) {
        exit(1);
    }
    int fd = open(argv[1], O_WRONLY);
    if (fd < 0) {return -1;}
    ssize_t value = write(fd, buffer, strlen(buffer));
    value++;
    close(fd);
    fd = open(argv[2], O_WRONLY);
    if (fd < 0) {exit(1);}
    while (1) {
        char message[MAX_MESSAGE];
        if (fgets(message, sizeof(message), stdin) == NULL) {
            close(fd);
            exit(1);
        }
        printf("as mensagens dentro do pub %s \n", message);
        message[strlen(message) - 1] = '\0';
        strcpy(buffer, "");
        messages_pipe newmessage;
        newmessage.code = 10;
        strcpy(newmessage.message, message);
        writer(&newmessage, newmessage.code, buffer);
        printf("mensagem serializada no pub %s\n",buffer);
        value = write(fd, buffer, sizeof(buffer));
        value++;
    }
    close(fd);
    fprintf(stderr, "usage: pub <register_pipe_name> <box_name>\n");
    WARN("unimplemented"); // TODO: implement
    return -1;
}
