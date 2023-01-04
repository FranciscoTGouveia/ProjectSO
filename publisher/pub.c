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
    request newrequest = {
        .code = 1,
        .pipe_name = argv[argc - 1],
        .box_name = argv[argc]
    };
    int file = open(GLOBAL_PATH, O_RDONLY);
    if (file < 0) {return -1;}
    char fifo_path[MAX_PIPE_NAME];
    read(file, fifo_path, sizeof(fifo_path));
    close(file);
    char buffer[MAX_LINE] = "";
    writer(&newrequest, newrequest.code, buffer);
    if (mkfifo(argv[argc-1], 0777) < 0) {
        exit(1);
    }
    int fd = open(fifo_path, O_WRONLY);
    if (fd < 0) {return -1;}
    write(fd, buffer, sizeof(buffer));
    close(fd);
    fd = open(argv[argc-1], O_WRONLY);
    if (fd < 0) {exit(1);}
    char message[MAX_MESSAGE];
    while (fgets(message, sizeof(message), stdin) != NULL) {
        strcpy(buffer, "");
        messages_pipe newmessage;
        newmessage.code = 10;
        strcpy(newmessage.message, message);
        writer(&newmessage, newmessage.code, buffer);
        write(fd, buffer, sizeof(buffer));
    }
    close(fd);
    fprintf(stderr, "usage: pub <register_pipe_name> <box_name>\n");
    WARN("unimplemented"); // TODO: implement
    return -1;
}
