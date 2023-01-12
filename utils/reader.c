#include <stdio.h>
#include <string.h>
#include "reader.h"
#include <stdlib.h>
#include <stdint.h>


/*manager_request_sring* reader_manager(char* buffer) {
    char *register_pipe, *pipe, *operation, *box;
    register_pipe = strtok(NULL, "|");
    pipe= strtok(NULL, "|");
    operation = strtok(NULL, "|");
    command_helper helper = {
        .register_name = register_pipe,
        .pipe_name = pipe,
        .box_name = NULL
    };
    manager_request_sring manager_h = {
        .operation = operation
    };
    if ((box = strtok(NULL, "|")) != NULL) {
        helper.box_name = box;
    }
    command_helper* helper_p = &helper;
    manager_request_sring* manager_p = &manager_h;
    manager_p->string = helper_p;
    return manager_p;
}*/


/*command_helper* reader_sub_pub(char* buffer) {
    char* register_pipe, *pipe, *box;
    register_pipe = strtok(NULL, "|");
    pipe = strtok(NULL, "|");
    box = strtok(NULL, "|");
    command_helper helper = {
        .register_name = register_pipe,
        .pipe_name = pipe,
        .box_name = box
    };
    command_helper* helper_p = &helper;
    return helper_p;
}*/

request* reader_request(uint8_t code_pipe) {
    request* request_pipe;
    request_pipe = malloc(sizeof(request));
    request_pipe->code = code_pipe;
    strcpy(request_pipe->pipe_name, strtok(NULL, "|"));
    strcpy(request_pipe->box_name, strtok(NULL, "|"));
    return request_pipe;
}

response_manager* reader_response_manager(uint8_t code_pipe) {
    response_manager* response;
    response = malloc(sizeof(response_manager));
    response->code = code_pipe;
    char* end;
    response->return_code = (int)strtoul(strtok(NULL,"|"), &end, 10);
    if (*end != '\0') {return NULL;}
    strcpy(response->error_message,strtok(NULL,"|"));
    return response;
}


list_manager_response* reader_list_response(uint8_t code_pipe) {
    list_manager_response* list_response;
    list_response = malloc(sizeof(list_manager_response));
    list_response->code = code_pipe;
    char* end;
    list_response->last =(uint8_t) strtoul(strtok(NULL, "|"), &end, 10);
    if (*end != '\0') {return NULL;}
    //list_response->last = (__uint8_t) strtok(NULL, "|");
    strcpy(list_response->box_name, strtok(NULL,"|"));
    list_response->box_size = strtoul(strtok(NULL,"|"), &end, 10);
    if (*end != '\0') {return NULL;}
    list_response->n_pubs = strtoul(strtok(NULL,"|"), &end, 10);
    if (*end != '\0') {return NULL;}
    char* teste = strtok(NULL,"|");
    if (teste == NULL) {
        list_response->n_subs = 0;
    } else {
        list_response->n_subs = strtoul(teste, &end, 10);
        if (*end != '\0') {return NULL;}
    }
    return list_response;
}

list_manager_request* reader_list_request(uint8_t code_pipe) {
    list_manager_request* list_request;
    list_request = malloc(sizeof(list_manager_request));
    list_request->code = code_pipe;
    strcpy(list_request->pipe_name, strtok(NULL,"|"));
    return list_request;
}

messages_pipe* reader_message(uint8_t code_pipe) {
    messages_pipe* message_request;
    message_request = malloc(sizeof(messages_pipe));
    message_request->code = code_pipe;
    strcpy(message_request->message, strtok(NULL, "|"));
    return message_request;
}


void *reader(uint8_t code_pipe) {
    void* message;
    switch (code_pipe) {
    case 4:
        message = reader_response_manager(code_pipe);
        break;
    case 6:
        message = reader_response_manager(code_pipe);
        break;
    case 7:
        message = reader_list_request(code_pipe);
        break;
    case 8:
        message = reader_list_response(code_pipe);
        break;
    case 9:
        message = reader_message(code_pipe);
        break;
    case 10:
        message = reader_message(code_pipe);
        break;
    default:
        message = reader_request(code_pipe);
        break;
    }
    return message;
}