#include "../utils/logging.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/task.h"
#include "../utils/reader.h"
#include "../utils/pipeflow.h"
#include "../producer-consumer/producer-consumer.h"
#include <unistd.h>

pc_queue_t *task_queue;
int active_threads = 0;
pthread_mutex_t thread_lock;
pthread_cond_t thread_cond;


void process_sub(request* arg) {
    return;
}

void process_pub(request* arg) {
    return;   
}

void process_manager(request* arg) {
    return;
}







void thread_init() {
    while (1) {
        pthread_mutex_lock(&thread_lock);
        task* newtask = pcq_dequeue(task_queue);
        pthread_mutex_unlock(&thread_lock);
        newtask->function(newtask->request);
        //while (1)
        // cond var.
        // read
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    if (mkfifo(GLOBAL_PATH, 0777) < 0) {exit(1);}
    if (mkfifo(argv[0], 0777) < 0) {exit(1);}
    int file = open(GLOBAL_PATH, O_WRONLY);
    if (file == -1) {return -1;}
    write(file,argv[argc - 1],strlen(GLOBAL_PATH) + 1);
    close(file);
    pthread_t thread_pool[atoi(argv[1])];
    task_queue = malloc(sizeof(pc_queue_t));
    pcq_create(task_queue, (size_t)atoi(argv[1]));
    if (pthread_mutex_init(&thread_lock, NULL) == -1) {return -1;}
    if (pthread_cond_init(&thread_lock, NULL) == -1) {return -1;}
    for (int i = 0; i < atoi(argv[1]); i++) {
        pthread_create(thread_pool[i], NULL, thread_init, NULL);
    }
    int fd = open(argv[0], O_RDONLY);
    if (fd == -1) {return -1;}
    while (1) {
        char buffer[MAX_LINE];
        read(fd, buffer, sizeof(buffer));
        __uint8_t code_pipe = (__uint8_t)strtok(buffer,"|");
        task* newtask;
        newtask = malloc(sizeof(task));
        newtask->request = reader(buffer,code_pipe);
        switch (code_pipe) {
            case 1:
                newtask->function = &process_pub;
                break;
            case 2:
                newtask->function = &process_sub;
                break;
            default:
                newtask->function = &process_manager;
                break;
        }
        if (pcq_enqueue(task_queue, newtask) == -1) {return -1;}
    }
    // handle signal to end the program and then join the threads
    fprintf(stderr, "usage: mbroker <pipename>\n");
    WARN("unimplemented"); // TODO: implement
    return -1;
}
