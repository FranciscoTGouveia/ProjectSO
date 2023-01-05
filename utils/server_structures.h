#ifndef __UTILS_SERVER_STRCUTURES__
#define __UTILS_SERVER_STRCUTURES__

#include <pthread.h>
#include "pipeflow.h"

typedef struct {
    pthread_t thread;
    int index; 
} thread;

typedef struct {
    char box_name[MAX_BOX_NAME];
    pthread_cond_t cond_var;
    int n_pub;
    int n_subs;
    int free; //0 if is free 1 if not
} box;


#endif