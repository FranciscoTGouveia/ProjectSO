#include "../utils/logging.h"
#include "../utils/pipeflow.h"
#include "../utils/betterassert.h"
#include "../utils/reader_stc.h"
#include "../utils/writer_stc.h"
#include "../utils/safety_mechanisms.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define CREATE "create"
#define REMOVE "remove"
static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> list\n");
}




int compare_func(const void*a, const void* b) {
    list_manager_response* first = *(list_manager_response**)a;
    list_manager_response* second = *(list_manager_response**)b;
    return strcmp(first->box_name, second->box_name);
}


void manager_request(void* newrequest,uint8_t code_pipe ,char* register_pipe) {
    char buffer[MAX_LINE];
    int fd;
    writer_stc(newrequest, code_pipe, buffer);
    if ((fd = open(register_pipe, O_WRONLY)) < 0) exit(1);
    if (write(fd, buffer, sizeof(buffer)) < 0) exit(1);
    close(fd);
}


void manager_create_remove(request* newrequest, char* pipe, char* register_pipe) {
    manager_request(newrequest,newrequest->code,register_pipe);
    int fd_fifo, fd;
    if ((fd_fifo = mkfifo(pipe, 0777)) < 0) exit(1);
    if ((fd = open(pipe, O_RDONLY)) < 0) exit(1);
    char message[MAX_LINE];
    if (read(fd, message, sizeof(message) < 0)) exit(1);
    response_manager* response = reader_stc(message);
    if (response->return_code == -1) {
        fprintf(stdout, "ERROR %s\n", response->error_message);
    } else {
        fprintf(stdout, "OK\n");
    }
    // Is this free really necessary ???
    free(response);
    if (close(fd) == -1) exit(1);
    unlink(pipe);
}



void manager_list(list_manager_request* newrequest, char* pipe, char*register_pipe) {
    manager_request(newrequest, newrequest->code, register_pipe);
    int fd_fifo, fd;
    if ((fd_fifo = mkfifo(pipe, 0777)) < 0) exit(1);
    size_t size = 100;
    int counter = 0;
    list_manager_response** list_of_boxes = my_malloc(size*sizeof(list_manager_response*));
    if ((fd = open(pipe, O_RDONLY)) < 0 ) exit(1);
    while (1) {
        // Will keep on reading through, until mbroker stops sending boxes list responses
        char message[MAX_LINE];
        memset(message, 0 ,MAX_LINE);
        if (read(fd, message, sizeof(message)) < 0) exit(1);
        list_of_boxes[counter] = reader_stc(message);
        if (list_of_boxes[counter]->last == 1) {
            if (list_of_boxes[counter]->box_name[0] == '\0') {
                fprintf(stdout, "NO BOXES FOUND\n");
                free(list_of_boxes[counter]);
                free(list_of_boxes);
                unlink(pipe);
                return;
            }
            break;
        }
        counter++;
        if (counter == size) { // Resize the array of boxes
            size *= 2;
            list_of_boxes = realloc(list_of_boxes, size*sizeof(list_manager_response*));
        }
    }
    if (close(fd) == -1) exit(1);
    //here we sort the array
    qsort(list_of_boxes, (size_t)(counter+1), sizeof(list_manager_response*), compare_func);
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
    if (argc == 5) { // Creation or deletion of a box
        request newrequest;
        strcpy(newrequest.pipe_name, argv[argc-3]);
        char box_name_slash[MAX_BOX_NAME];
        memset(box_name_slash, 0, MAX_BOX_NAME);
        strcpy(box_name_slash, "/");
        strcat(box_name_slash, argv[argc-1]);
        strcpy(newrequest.box_name, box_name_slash);
        if (strcmp(argv[argc-2], CREATE) == 0) {
            newrequest.code = 3;
        } else if (strcmp(argv[argc-2], REMOVE) == 0) {
            newrequest.code = 5;
        }
        manager_create_remove(&newrequest, argv[argc-3], argv[argc-4]);
    } else if (argc == 4) { // Listing of all boxes
        list_manager_request newrequest;
        strcpy(newrequest.pipe_name, argv[argc - 2]);
        newrequest.code = 7;
        manager_list(&newrequest, argv[argc-2], argv[argc-3]);
    } else { print_usage(); }
    return 0;
}
