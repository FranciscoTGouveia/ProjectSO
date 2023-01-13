#include "../utils/logging.h"
#include "../utils/pipeflow.h"
#include "../utils/betterassert.h"
#include "../utils/reader_stc.h"
#include "../utils/writer_stc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define CREATE "create"
#define REMOVE "remove"
/*static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> list\n");
}*/


void sort_boxes(list_manager_response** list_of_boxes, int counter) {
    for(int i = 0; i < counter; i++) {
        for(int j = 0; j < counter-i; i++) {
            if (strcmp(list_of_boxes[i]->box_name, list_of_boxes[i+1]->box_name) > 0) {
                void* temp = list_of_boxes[i];
                list_of_boxes[i] = list_of_boxes[i+1];
                list_of_boxes[i+1] = temp;
            }
        }
    }
}


void manager_request(void* newrequest,uint8_t code_pipe ,char* register_pipe) {
    char buffer[MAX_LINE];
    writer_stc(newrequest, code_pipe, buffer);
    printf("buffer a ser enviado no pipe pelo manager %s\n", buffer);
    int fd = open(register_pipe, O_WRONLY);
    if (fd < 0) {exit(1);}
    printf("Tamanho do buffer do manager %ld\n", strlen(buffer));
    ssize_t value = write(fd, buffer, sizeof(buffer));
    printf("Tamanho do q foi escrito noo manager %ld\n", value);
    value++;
    close(fd);
}


void manager_create_remove(request* newrequest, char* pipe, char* register_pipe) {
    manager_request(newrequest,newrequest->code,register_pipe);
    int fd_fifo;
    if ((fd_fifo = mkfifo(pipe, 0777)) < 0) {exit(1);}
    int fd = open(pipe, O_RDONLY);
    if (fd < 0) {exit(1);}
    char message[MAX_LINE];
    ssize_t value = read(fd, message, sizeof(message));
    printf("dps do reader %s \n", message);
    value++;
    //char* end;
    //uint8_t code_pipe =(uint8_t)strtoul(strtok(message, "|"), &end, 10);
    response_manager* response = reader_stc(message);
    printf("valores da response code %d message %s return %d \n", response->code, response->error_message, response->return_code);
    if (response->return_code == -1) {
        fprintf(stdout, "ERROR %s\n", response->error_message);
    } else {
        fprintf(stdout, "OK\n");
    }
    free(response);
    close(fd);
    unlink(pipe);
}

void manager_list(list_manager_request* newrequest, char* pipe, char*register_pipe) {
    manager_request(newrequest, newrequest->code, register_pipe);
    int fd_fifo;
    printf("antes do mkfifo \n");
    if ((fd_fifo = mkfifo(pipe, 0777))  < 0) {exit(1);}
    long unsigned int size = 100;
    int counter = 0;
    list_manager_response** list_of_boxes = malloc(size*sizeof(list_manager_response*));
    int fd = open(pipe, O_RDONLY);
    if (fd < 0) {exit(1);}
    while (1) {
        char message[MAX_LINE];
        memset(message, 0 ,MAX_LINE);
        printf("dps do memset\n");
        ssize_t value = read(fd, message, sizeof(message));
        printf("esta e a message %s \n", message);
        value++;
        //char* end;
        //printf("antes do strtok \n");
        //uint8_t code_pipe =(uint8_t)strtoul(strtok(message, "|"), &end, 10);
        printf("dps do strtok \n");
        list_of_boxes[counter] = reader_stc(message);
        printf("dps do reader \n");
        if (list_of_boxes[counter]->last == 1) {
            printf("antes do box_name\n");
            if (list_of_boxes[counter]->box_name[0] == '0') {
                fprintf(stdout, "NO BOXES FOUND\n");
                free(list_of_boxes[counter]);
                free(list_of_boxes);
                unlink(pipe);
                return;
            }
            break;
        }
        counter++;
        if (counter == size) {
            size *= 2;
            list_of_boxes = realloc(list_of_boxes, size*sizeof(list_manager_response*));
        }
    }
        close(fd);
    //here we sort the array
    sort_boxes(list_of_boxes, counter);
    for (int i = 0; i <= counter; i++) {
        fprintf(stdout, "%s %zu %zu %zu\n", list_of_boxes[i]->box_name, 
        list_of_boxes[i]->box_size, list_of_boxes[i]->n_pubs, list_of_boxes[i]->n_subs);
    }
    for (int i = 0; i <= counter; i++) {
        free(list_of_boxes[i]);
    }
    free(list_of_boxes);
    close(fd_fifo);
    unlink(pipe);
}



int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    if (argc == 5) {
        request newrequest;
        strcpy(newrequest.pipe_name, argv[argc - 3]);
        strcpy(newrequest.box_name, argv[argc - 1]);
        if (strcmp(argv[argc - 2],CREATE) == 0) {
            newrequest.code = 3;
        } else if (strcmp(argv[argc - 2], REMOVE) == 0) {
            newrequest.code = 5;
        }
        manager_create_remove(&newrequest, argv[argc-3], argv[argc-4]);
    } else {
        list_manager_request newrequest;
        strcpy(newrequest.pipe_name, argv[argc - 2]);
        newrequest.code = 7;
        manager_list(&newrequest, argv[argc-2], argv[argc-3]);
    }
    return 0;
}
