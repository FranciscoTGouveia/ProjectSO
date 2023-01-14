#include "producer-consumer.h"
#include <pthread.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "../utils/task.h"
#include "../utils/pipeflow.h"


int pcq_create(pc_queue_t* queue, size_t capacity) {
    queue->pcq_capacity = capacity;
    queue->pcq_buffer = malloc(sizeof(task)*capacity);// check malloc
    pthread_mutex_init(&queue->pcq_current_size_lock, NULL); //check mutex
    pthread_mutex_init(&queue->pcq_head_lock, NULL);
    pthread_mutex_init(&queue->pcq_tail_lock, NULL);
    pthread_mutex_init(&queue->pcq_pusher_condvar_lock, NULL);
    pthread_mutex_init(&queue->pcq_popper_condvar_lock, NULL);
    pthread_cond_init(&queue->pcq_pusher_condvar, NULL);
    pthread_cond_init(&queue->pcq_popper_condvar, NULL);
    return 0;
}


//Check other fucntions
int pcq_destroy(pc_queue_t *queue) {
    printf("DESTROY\n");
    pthread_mutex_destroy(&queue->pcq_current_size_lock);
    pthread_mutex_destroy(&queue->pcq_head_lock);
    pthread_mutex_destroy(&queue->pcq_tail_lock);
    pthread_mutex_destroy(&queue->pcq_pusher_condvar_lock);
    pthread_mutex_destroy(&queue->pcq_popper_condvar_lock);
    pthread_cond_destroy(&queue->pcq_pusher_condvar);
    pthread_cond_destroy(&queue->pcq_popper_condvar);
    for (size_t i = queue->pcq_head; i <= queue->pcq_tail; i++) {
        free(((task*)queue->pcq_buffer[i])->request);
        free(queue->pcq_buffer[i]);
    }
    free(queue->pcq_buffer);
    return 0;
}

int pcq_enqueue(pc_queue_t *queue, void* elem) {
    pthread_mutex_lock(&queue->pcq_pusher_condvar_lock);
    while (queue->pcq_current_size == queue->pcq_capacity) {
        pthread_cond_wait(&queue->pcq_pusher_condvar,
        &queue->pcq_pusher_condvar_lock);
    }
    pthread_mutex_lock(&queue->pcq_current_size_lock);
    pthread_mutex_lock(&queue->pcq_head_lock);
    /*if (queue->pcq_tail == -1) {
        queue->pcq_buffer[queue->pcq_head] = elem;
        queue->pcq_tail++;
        queue->pcq_current_size++;
        pthread_mutex_unlock(&queue->pcq_head_lock);
        pthread_mutex_unlock(&queue->pcq_current_size_lock);
        pthread_mutex_unlock(&queue->pcq_pusher_condvar_lock);
        pthread_cond_signal(&queue->pcq_popper_condvar);
        return 0;
    }*/
    if (queue->pcq_current_size != 0) {
        for (size_t i = queue->pcq_tail + 1; i > queue->pcq_head; i--) {
            queue->pcq_buffer[i] = queue->pcq_buffer[i-1];
        }
    }
    queue->pcq_buffer[queue->pcq_head] = elem;
    if (queue->pcq_current_size != 0) {
        queue->pcq_tail++;
    }
    queue->pcq_current_size++;
    pthread_mutex_unlock(&queue->pcq_head_lock);
    pthread_mutex_unlock(&queue->pcq_current_size_lock);
    pthread_mutex_unlock(&queue->pcq_pusher_condvar_lock);
    pthread_cond_signal(&queue->pcq_popper_condvar);
    printf("Fim do push\n");
    printf("Ainda dentro do push saber o nome da box %s\n", ((request*)((task*)queue->pcq_buffer[queue->pcq_head])->request)->box_name);
    return 0;        
}


void* pcq_dequeue(pc_queue_t* queue) {
    pthread_mutex_lock(&queue->pcq_popper_condvar_lock);
    printf("Entramos no pop\n");
    while (queue->pcq_current_size == 0) {
        printf("Estamos presos no pop\n");
        pthread_cond_wait(&queue->pcq_popper_condvar,
         &queue->pcq_popper_condvar_lock);
    }
    pthread_mutex_lock(&queue->pcq_current_size_lock);
    pthread_mutex_lock(&queue->pcq_tail_lock);
    printf("Dentro do pop a tail é %ld \n", queue->pcq_tail);
    task* removed_task = (task*) queue->pcq_buffer[queue->pcq_tail];

    printf("dentro do pop ver o nome da box %s \n", ((request*)removed_task->request)->box_name);
    //free(((task*)queue->pcq_buffer[queue->pcq_tail])->request);
    //free(queue->pcq_buffer[queue->pcq_tail]);
    //queue->pcq_buffer[queue->pcq_tail] = NULL;
    if (queue->pcq_tail != queue->pcq_head) {
        queue->pcq_tail--;
    }
    queue->pcq_current_size--;
    pthread_mutex_unlock(&queue->pcq_tail_lock);
    pthread_mutex_unlock(&queue->pcq_current_size_lock);
    pthread_mutex_unlock(&queue->pcq_popper_condvar_lock);
    pthread_cond_signal(&queue->pcq_pusher_condvar);
    return removed_task;
}