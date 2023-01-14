#include "../utils/logging.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/task.h"
#include "../utils/reader_stc.h"
#include "../utils/writer_stc.h"
#include "../utils/pipeflow.h"
#include "../utils/server_structures.h"
#include "../producer-consumer/producer-consumer.h"
#include <unistd.h>
#include "../fs/operations.h"
#include <errno.h>
#include <signal.h>
#define  END 1
#define ON_GOING 0

pc_queue_t *task_queue;
int n_threads;
char* server_pipe;
pthread_mutex_t thread_lock;
pthread_cond_t thread_cond;
box* server_boxes;
int size_boxes;
pthread_mutex_t box_size_lock;
thread* thread_pool;
int status = ON_GOING;





void ignore_signal(int s) {
   ssize_t t = write(1,"ESTOU NO SGINAL",strlen("ESTOU NO SGINAL"));
   (void) t;
   (void) s;
   signal(SIGPIPE, ignore_signal);
} // we can improve this by using a mask

void end_program_ctrlC(int s) {
    ssize_t value = write(1, "O PROGRAMA IRÁ ENCERRAR ASSIM QUE POSSÍVEL\n",
    strlen("O PROGRAMA IRÁ ENCERRAR ASSIM QUE POSSÍVEL\n"));
    status = END;
    //pcq_destroy(task_queue);
    free(task_queue);
    free(thread_pool);
    free(server_boxes);
    unlink(server_pipe);
    printf("O PROGRAMA ENCERROU\n");
    exit(0);
    (void) value;
    (void) s;
}



void process_sub(void* arg, int* index) {
    signal(SIGINT, end_program_ctrlC);
    pthread_mutex_lock(&box_size_lock);
    printf("entramos no sub\n");
    int tester = 0;
    for (int i = 0; i < size_boxes; i++) {
        if (strcmp(((request*)arg)->box_name, server_boxes[i].box_name) == 0) {
            tester = 1;
            thread_pool[*index].index = i;
            server_boxes[i].n_subs += 1;
            printf("achou a caixa\n");
            break;
        } 
    }
    pthread_mutex_unlock(&box_size_lock);
    if (tester == 0) {
        printf("deu exit\n");
        int fd = open(((request*)arg)->pipe_name, O_WRONLY);
        if (fd < 0) {printf("exit2 deu bronca \n");exit(1);}
        close(fd);
        return;
    }
    int fd_tfs = tfs_open(((request*)arg)->box_name, 0);
    if (fd_tfs == -1) {printf("aconteceu algo a abrir\n");exit(1);}
    signal(SIGPIPE, ignore_signal);
    int fd = open(((request*)arg)->pipe_name, O_WRONLY);
    if (fd < 0) {printf("exit2 deu bronca \n");exit(1);}
    while (1) {
        pthread_mutex_lock(&server_boxes[thread_pool[*index].index].box_lock);
        messages_pipe newmessage;
        newmessage.code = 10;
        char teste[1024];
        ssize_t value;
        while ((value = tfs_read(fd_tfs, teste, sizeof(teste))) == 0) {
            printf("estou bloqueado no sub\n");
            pthread_cond_wait(&server_boxes[thread_pool[*index].index].cond_var,
             &server_boxes[thread_pool[*index].index].box_lock);
        }
        printf("ESTOU DESBLOQUEADO \n");
        if (status == END) {
            pthread_mutex_unlock(&server_boxes[thread_pool[*index].index].box_lock);
            close(fd);
            tfs_close(fd_tfs);
            return;
        }
        if (value == -1) {
            server_boxes[thread_pool[*index].index].n_subs -=1;
            close(fd);
            tfs_close(fd_tfs);
            pthread_mutex_unlock(&server_boxes[thread_pool[*index].index].box_lock);
            return;
        }
        while (value > 0) {
            printf("valor do value %ld\n", value);
            strcpy(newmessage.message, teste);
            printf("teste %s\n", teste);
            printf("bytes no read do sub %s\n", newmessage.message);
            char pipe_message[MAX_LINE];
            writer_stc(&newmessage,10, pipe_message);
            printf("dps do writer no sub %s\n",pipe_message );
            printf("dps do open no sub\n");
            ssize_t bytes = write(fd, pipe_message, sizeof(pipe_message));
            printf("dps do write no sub\n");
            if (bytes == -1) {
                if (errno == EPIPE) {
                    printf("entrei no Epipe\n");
                    server_boxes[thread_pool[*index].index].n_subs -=1;
                    tfs_close(fd_tfs);
                    pthread_mutex_unlock(&server_boxes[thread_pool[*index].index].box_lock);
                    return;
                }
            }
            memset(teste, 0, sizeof(teste));
            memset(newmessage.message, 0, (sizeof(char)*1024));
            value = tfs_read(fd_tfs, teste, sizeof(teste));
            printf("valor do value dps do segundo read %ld\n", value);
            if (value == -1) {
                server_boxes[thread_pool[*index].index].n_subs -=1;
                close(fd);
                tfs_close(fd_tfs);
                pthread_mutex_unlock(&server_boxes[thread_pool[*index].index].box_lock);
                return;
            }
            printf("nao dei exit no sub\n");
        }
        pthread_mutex_unlock(&server_boxes[thread_pool[*index].index].box_lock);
    }
    close(fd);
    tfs_close(fd_tfs);
    server_boxes[thread_pool[*index].index].n_subs -=1;
    return;
}

void process_pub(void* arg, int* index) {
    signal(SIGINT, end_program_ctrlC);
    pthread_mutex_lock(&box_size_lock);
    int tester = 0;
    for (int i = 0; i < size_boxes; i++) {
        if (strcmp(((request*)arg)->box_name, server_boxes[i].box_name) == 0) {
            if (server_boxes[i].n_pub == 1) {
                pthread_mutex_unlock(&box_size_lock);
                int fd = open(((request*)arg)->pipe_name, O_RDONLY);
                if (fd < 0) {printf("teste exit1\n");exit(1);}
                close(fd);
                printf("ENTROU AQUI\n");
                return;
            }
            tester = 1;
            thread_pool[*index].index = i;
            server_boxes[i].n_pub = 1;
            server_boxes[i].box_size = 0;
            printf("Achou a caixa no pub\n");
            break;
        } 
    }
    if (tester == 0) {
        printf("teste exit0\n");
        int fd = open(((request*)arg)->pipe_name, O_RDONLY);
        if (fd < 0) {printf("teste exit1\n");exit(1);}
        close(fd);
        pthread_mutex_unlock(&box_size_lock);
        return;
    }
    pthread_mutex_unlock(&box_size_lock);
    signal(SIGPIPE, ignore_signal);
    int fd = open(((request*)arg)->pipe_name, O_RDONLY);
    if (fd < 0) {printf("teste exit1\n");exit(1);}
    int fd_tfs = tfs_open(((request*)arg)->box_name, TFS_O_APPEND);
    if (fd_tfs == -1) {
        printf("teste exit2\n");
        close(fd);
        return;
    }
    while (1) {
        if (status == END) {
            close(fd);
            tfs_close(fd_tfs);
            return;
        }
        printf("ESTOU NO LOOP DO PUB\n");
        printf("este e o erno %d", errno);
        char buffer[MAX_LINE];
        printf("teste1\n");
        ssize_t bytes = read(fd,buffer,sizeof(buffer));
        printf("este e o erno dps do read%d\n", errno);
            if (bytes == 0) {
               break; 
            }
        if (bytes > 0) {
            //char *end;
            uint8_t teste;
            memcpy(&teste, buffer, sizeof(teste));
            printf("Isto e o code do meu buffer %d\n", teste);
            printf("OLA BRO N ME IGNORES\n");
            //uint8_t code_pipe =(uint8_t)strtoul(strtok(buffer, "|"), &end, 10);
            messages_pipe* pipe_message = reader_stc(buffer);
            ssize_t bytes_tfs = tfs_write(fd_tfs, pipe_message->message,
             strlen(pipe_message->message) + 1);
            if (bytes_tfs == -1) {
                printf("demos exit\n");
                free(pipe_message);
                break;
            }
            server_boxes[thread_pool[*index].index].box_size += (int)strlen(pipe_message->message);
            printf("tamanho do q foi escrito pelo pub %ld\n", bytes_tfs);
            printf("tamanho do q era suposto ter escrito %ld\n", sizeof(pipe_message->message));
            printf("mensagem q foi escrita %s \n", pipe_message->message);
            pthread_cond_broadcast(&server_boxes[thread_pool[*index].index].cond_var);
            /*char teste[MAX_LINE];
            fd_tfs = tfs_open(((request*)arg)->box_name, TFS_O_APPEND);
            if (fd_tfs == -1) {printf("teste exit2\n");exit(1);}
            ssize_t b = tfs_read(fd_tfs, teste, strlen(teste));
            printf("tamanho do read %ld \n", b);
            printf("buffer lido no tfs %s \n", teste);
            b = tfs_read(fd_tfs, teste, strlen(teste));
            printf("tamanho do read %ld \n", b);
            printf("buffer lido no tfs %s \n", teste);
            tfs_close(fd_tfs);
            b++;*/
            free(pipe_message);
        }
    }
    tfs_close(fd_tfs);
        close(fd);
    printf("sai do loop no pub \n");
    server_boxes[thread_pool[*index].index].n_pub = 0;
    return;   
}


void process_manager_list(void* arg, int* index) {
    signal(SIGINT, end_program_ctrlC);
    (void)index;
    printf("entrou no list\n");
    pthread_mutex_lock(&box_size_lock);
    int counter = 0;
    list_manager_response boxes_to_send[size_boxes];
    for (int i = 0; i < size_boxes; i++) {
        if (server_boxes[i].free == 1) {
            boxes_to_send[counter].code = 8;
            boxes_to_send[counter].last = 0;
            strcpy(boxes_to_send[counter].box_name, server_boxes[i].box_name);
            boxes_to_send[counter].box_size = (uint64_t)server_boxes[i].box_size; // need to calculate the size 
            boxes_to_send[counter].n_pubs = (unsigned int)server_boxes[i].n_pub;
            boxes_to_send[counter].n_subs = (unsigned int)server_boxes[i].n_subs;
            counter++;
        }
    }
    pthread_mutex_unlock(&box_size_lock);
    if (counter == 0) {
        boxes_to_send[counter].code = 8;
            boxes_to_send[counter].n_subs = 24; 
        memset(boxes_to_send[counter].box_name, 0, MAX_BOX_NAME);
        counter++;
    }
    boxes_to_send[counter - 1].last = 1;
    printf("ANTES DO OPEN \n");
    signal(SIGPIPE, ignore_signal);
    int fd = open(((list_manager_request*)arg)->pipe_name, O_WRONLY);
    if (fd < 0) {
        return;
    }
    printf("valor de counter %d \n", counter);
    for (int i = 0; i < counter; i++) {
        printf("dentro do loop %s last %d \n", boxes_to_send[i].box_name,
        boxes_to_send[i].last);
        char buffer[MAX_LINE];
        writer_stc(&boxes_to_send[i], boxes_to_send[i].code, buffer);
        printf("valor do writer %s\n",buffer);
        ssize_t value = write(fd, buffer, sizeof(buffer));
        if (value == -1) {
            if (errno == EPIPE) {
                break;
            }
        }
        value++;
    }
    close(fd);
    
}



void process_manager_remove(void* arg, int* index) {
    signal(SIGINT, end_program_ctrlC);
    pthread_mutex_lock(&box_size_lock);
    int tester = 0;
    for (int i = 0; i < size_boxes; i++) {
        if (strcmp(((request*)arg)->box_name, server_boxes[i].box_name) == 0) {
            if (server_boxes[i].free == 0) {
                pthread_mutex_unlock(&box_size_lock);
                return;
            }
            thread_pool[*index].index = i;
            tester = 1;
            break;
        }
    }
    if (tester == 0) {tester = -1;}
    tester = 0;
    response_manager response;
    response.code = 6;
    tester = tfs_unlink(((request*)arg)->box_name);
    if (tester == -1) {
        response.return_code = -1;
        strcpy(response.error_message, "Ocorreu um erro ao eliminar a caixa");
    } else {
        response.return_code = 0;
        memset(response.error_message, 0, 1024);
        server_boxes[thread_pool[*index].index].free = 0;
        server_boxes[thread_pool[*index].index].n_pub = 0;
        server_boxes[thread_pool[*index].index].n_subs = 0;
        server_boxes[thread_pool[*index].index].box_size = 0;
        memset(server_boxes[thread_pool[*index].index].box_name, 0, MAX_BOX_NAME);
    }
    char buffer[MAX_LINE];
    writer_stc(&response, response.code, buffer);
    printf("writer do manager remove %s %ld\n", buffer, strlen(buffer));
    signal(SIGPIPE, ignore_signal);
    int fd = open(((request*)arg)->pipe_name, O_WRONLY);
    if (fd < 0) {
        pthread_mutex_unlock(&box_size_lock);
        return;
    }
    ssize_t value = write(fd,buffer, sizeof(buffer));
    if (value == -1) {
        if (errno == EPIPE) {
            pthread_mutex_unlock(&box_size_lock);
            return;
        }
    }
    printf("quanto foi o meu value %ld\n", value);
    value++;
    pthread_mutex_unlock(&box_size_lock);
    close(fd);
    pthread_cond_broadcast(&server_boxes[thread_pool[*index].index].cond_var);
}



void process_manager_create(void* arg, int* index) {
    signal(SIGINT, end_program_ctrlC);
    printf("cheguei manager\n");
    pthread_mutex_lock(&box_size_lock);
    printf("index no manager %d\n", *index);
    int tester = 0;
    for (int i = 0; i < size_boxes; i++) {
        if (strcmp(((request *)arg)->box_name, server_boxes[i].box_name) == 0) {
            printf("JA EXISTE COM O MESMO NOME \n");
            signal(SIGPIPE, ignore_signal);
            int fd = open(((request*)arg)->pipe_name, O_WRONLY);
            if (fd < 0) {printf("deu exit\n");exit(1);}
            response_manager response;
            response.code = 4;
            response.return_code = -1;
            strcpy(response.error_message, "Ocorreu um erro na criação da caixa");
            pthread_mutex_unlock(&box_size_lock);
            char buffer[MAX_LINE];
            writer_stc(&response, response.code, buffer);
            ssize_t bytes = write(fd, buffer, sizeof(buffer));
            if (bytes == -1) {
                if (errno == EPIPE) {
                    return;
                }
            }
            close(fd);
            return;
        }   
    }
    for (int i = 0; i < size_boxes; i++) {
        if (server_boxes[i].free == 0 ) {
            tester = 1;
            thread_pool[*index].index = i;
            server_boxes[i].free = 1;
            strcpy(server_boxes[i].box_name, ((request*)arg)->box_name);
            break;
        }
    }
    if (tester == 0) {
        server_boxes = realloc(server_boxes, sizeof(box)*2*(unsigned int)size_boxes);
        for (int i = size_boxes; i < (2*size_boxes);i++) {
            server_boxes[i].free = 0;
            if (pthread_cond_init(&server_boxes[i].cond_var, NULL) == -1) {exit(1);}
            if (pthread_mutex_init(&server_boxes[i].box_lock, NULL) == -1) {exit(1);}

        }
        thread_pool[*index].index = size_boxes;
        server_boxes[size_boxes].free = 1;
        strcpy(server_boxes[size_boxes].box_name, ((request*)arg)->box_name);
        size_boxes*=2;
    }
    tester = 0;
    printf("box name  dentro do process manager %s\n", ((request*)arg)->box_name);
    int fd_tfs = tfs_open(((request*)arg)->box_name, TFS_O_CREAT);
    if (fd_tfs == -1) {
        printf("deu merda\n");
        tester = 1;
    }
    tfs_close(fd_tfs);
    signal(SIGPIPE, ignore_signal);
    int fd = open(((request*)arg)->pipe_name, O_WRONLY);
    if (fd < 0) {printf("demos exit\n");exit(1);}
    response_manager response;
    response.code = 4;
    if (tester == 1) {
        server_boxes[thread_pool[*index].index].free = 0; //if you cant create the box it really isnt free
        memset(server_boxes[thread_pool[*index].index].box_name, 0, MAX_BOX_NAME);
        response.return_code = -1;
        strcpy(response.error_message, "Ocorreu um erro na criação da caixa");
    } else {
        response.return_code = 0;
        memset(response.error_message, 0, 1024);
    }
    pthread_mutex_unlock(&box_size_lock);
    char buffer[MAX_LINE];
    writer_stc(&response, response.code, buffer);
    ssize_t bytes = write(fd, buffer, sizeof(buffer));
    if (bytes == -1) {
        if (errno == EPIPE) {
            return;
        }
    }
    printf("tamanho de mbroker para manager %ld \n", bytes);
    bytes++;
    close(fd);
    return;
}






void *thread_init(void*  index) {
    signal(SIGINT, end_program_ctrlC);
    while (1) {
        printf("o valor do index antes de qualquer cena init %d \n", *(int*)index);
        printf("estou no init\n");
        task* newtask = pcq_dequeue(task_queue);
        printf("demos pop\n");
        printf("dps do pop ver o noma da box %s\n", ((request*)newtask->request)->box_name);
        printf("valor do index no init %d \n",*((int*)index));
        newtask->function(newtask->request, (int *)index);
        free(newtask->request);
        free(newtask);
        printf("SOU UMA THREAD E EU ACABEI\n");
        if (status == END) {
            return NULL;
        }
        //newtask need to be freed
        //while (1)
        // cond var.
        // read
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    signal(SIGINT, end_program_ctrlC);
    tfs_init(NULL);
    n_threads = atoi(argv[2]);
    server_pipe = argv[1];
    if (mkfifo(argv[1], 0777) < 0) {exit(1);}
    server_boxes = malloc((unsigned int)atoi(argv[2])*sizeof(box));
    size_boxes = atoi(argv[2]);
    thread_pool = malloc((unsigned int)atoi(argv[2])*sizeof(thread));
    task_queue = malloc(sizeof(pc_queue_t));
    pcq_create(task_queue, (size_t)atoi(argv[2]));
    printf("fora do create capacidade %ld\n", task_queue->pcq_capacity);
    if (pthread_mutex_init(&box_size_lock, NULL) == -1) {return -1;}
    if (pthread_mutex_init(&thread_lock, NULL) == -1) {return -1;}
    if (pthread_cond_init(&thread_cond, NULL) == -1) {return -1;}
    int index;
    for (int i = 0; i < atoi(argv[2]); i++) {
        thread_pool->index = -1;
        index = i;
        printf("valor do index %d \n", index);
        pthread_create(&thread_pool->thread, NULL, thread_init, (void *)&index);
        server_boxes[i].free = 0;
        if (pthread_cond_init(&server_boxes[i].cond_var, NULL) == -1) {return -1;}
        if (pthread_mutex_init(&server_boxes[i].box_lock, NULL) == -1) {return -1;}
    }
    while (1) {
        /*if (status == GOING_TO_END) {
            printf("entrou aqui\n");
            status = END;
        }*/
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {return -1;}
        char buffer[MAX_LINE];
        ssize_t value = read(fd, buffer, (sizeof(char)*MAX_LINE));//MELHOR TIRAR O READ E BLOQUEANTE
        (void) value;
        printf("li no mbroker\n");
        printf("Aqui e o buffer lido %s\n", buffer);
        //char* end;
        //uint8_t code_pipe =(uint8_t)strtoul(strtok(buffer, "|"), &end, 10);
        //printf("%u\n",code_pipe);
        //uint8_t code_pipe = (uint8_t)strtok(buffer,"|");
        task* newtask;
        newtask = malloc(sizeof(task));
        newtask->request = reader_stc(buffer);
        printf("buffer do mbroker dps do reader code %d pipe %s box %s\n", ((request*)newtask->request)->code,
        ((request*)newtask->request)->pipe_name, ((request*)newtask->request)->box_name);
        uint8_t code_pipe;
        memcpy(&code_pipe, buffer, sizeof(uint8_t));
        printf("code pipe no mbroker %d\n", code_pipe);
        printf("tetatva de ver o box mbroker %s \n", ((request*)newtask->request)->box_name);
        switch (code_pipe) {
            case 1:
                newtask->function = &process_pub;
                break;
            case 2:
                newtask->function = &process_sub;
                break;
            case 3:
                newtask->function = &process_manager_create;
                break;
            case 5:
                newtask->function = &process_manager_remove;
                break; 
            case 7:
                newtask->function = &process_manager_list;
                break;
            default:
                break;
        }
        printf("damos push aqui\n");
        if (pcq_enqueue(task_queue, newtask) == -1) {return -1;}
        memset(buffer, 0, sizeof(buffer));
        close(fd);
    }
    // handle signal to end the program and then join the threads
    fprintf(stderr, "usage: mbroker <pipename>\n");
    WARN("unimplemented"); // TODO: implement
    return -1;
}
