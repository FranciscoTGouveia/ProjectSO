#ifndef __UTILS_PIPEFLOW__
#define __UTILS_PIPEFLOW__

#include <stdint.h>
#include <math.h>
#define MAX_PIPE_NAME 256
#define MAX_BOX_NAME 32 
#define MAX_LINE 572 
#define GLOBAL_PATH "global_fifo"

typedef struct __attribute__ ((__packed__)){
    uint8_t code;
    char pipe_name[MAX_PIPE_NAME];
    char box_name[MAX_BOX_NAME];
} request;

typedef struct __attribute__ ((__packed__)){
    uint8_t code;
    char message[1024];
} messages_pipe;

typedef struct __attribute__ ((__packed__)){
    uint8_t code;
    int32_t return_code;
    char error_message[1024];
} response_manager;

typedef struct __attribute__ ((__packed__)){
    uint8_t code;
    uint8_t last;
    char box_name[MAX_BOX_NAME];
    uint64_t box_size;
    uint64_t n_pubs;
    uint64_t n_subs;
} list_manager_response;


typedef struct __attribute__ ((__packed__)){
    uint8_t code;
    char pipe_name[MAX_PIPE_NAME];
} list_manager_request;
#endif