#ifndef __UTILS_READER_H__
#define __UTILS_READER_H__
#include "pipeflow.h"


typedef struct {
    char* register_name;
    char* pipe_name;
    char* box_name;
} command_helper;

typedef struct {
    command_helper* string;
    char* operation;

} manager_request_sring;



// manager_request_sring* reader_manager(char* buffer);
//command_helper* reader_sub_pub(char* buffer);
void* reader(uint8_t code_pipe);


#endif  