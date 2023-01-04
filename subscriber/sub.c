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


int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    int file = open(GLOBAL_PATH, O_RDONLY);
    if (file < 0) {exit(1);}
    char global_fifo[MAX_PIPE_NAME];
    read(file, global_fifo, sizeof(global_fifo));
    close(file);
    request newrequest = {
        .code = 2,
        .pipe_name = argv[argc - 1],
        .box_name = argv[argc]
    };
    char buffer[MAX_LINE] = "";
    writer(&newrequest, newrequest.code, buffer);
    int fd = open(global_fifo, O_WRONLY);
    write(fd, buffer, sizeof(buffer));
    close(fd);
    if (mkfifo(argv[argc-1], 0777) < 0) {
        exit(1);
    }
    fd = open(argv[argc-1], O_RDONLY);
    if (fd < 0) {exit(1);}
    while (1) {
        char message[MAX_LINE];
        read(fd, message, sizeof(message));
        messages_pipe* newmesage = reader(message, 10);
        fprintf(stdout, "%s\n", newmesage->message);
        free(newmesage);
    }
    fprintf(stderr, "usage: sub <register_pipe_name> <box_name>\n");
    WARN("unimplemented"); // TODO: implement
    return -1;
}
