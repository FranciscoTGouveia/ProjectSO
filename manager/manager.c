#include "../utils/logging.h"
#include "../utils/pipeflow.h"
#include "../utils/betterassert.h"
#include "../utils/reader.h"
#include "../utils/writer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> list\n");
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    request newrequest;
    newrequest.code = 3;
    strcpy(newrequest.pipe_name, argv[argc - 3]);
    strcpy(newrequest.box_name, argv[argc - 1]);
    char buffer[MAX_LINE] = "";
    writer(&newrequest, newrequest.code, buffer);
    printf("buffer a ser enviado no pipe pelo manager %s\n", buffer);
    if (mkfifo(argv[argc- 3], 0777) < 0) {exit(1);}
    int fd = open(argv[argc-4], O_WRONLY);
    if (fd < 0) {return -1;}
    printf("Tamanho do buffer do manager %ld\n", strlen(buffer));
    ssize_t value = write(fd, buffer, strlen(buffer));
    printf("Tamanho do q foi escrito noo manager %ld\n", value);
    value++;
    close(fd);
    fd = open(argv[argc-3], O_RDONLY);
    if (fd < 0) {exit(1);}
    char message[MAX_LINE];
    value = read(fd, message, sizeof(message));
    while (value == 0) {
        value = read(fd, message, sizeof(message));
    }
    printf("%s", message);
    close(fd);
    return 0;
    strtok(message, "|");
    response_manager* response = reader(4);
    printf("%s", response->error_message);
    free(response);
    
    print_usage();
    WARN("unimplemented"); // TODO: implement
    return -1;
}
