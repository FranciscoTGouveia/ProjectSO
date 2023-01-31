#include "reader_stc.h"
#include <string.h>
#include <stdlib.h>
#include "writer_stc.h"
#include "pipeflow.h"
#include <stdio.h>
#include "safety_mechanisms.h"

request *reader_stc_request(uint8_t code_pipe, char buffer[MAX_LINE]) {
    request *request_pipe = my_malloc(sizeof(request));
    request_pipe->code = code_pipe;
    size_t offset = sizeof(uint8_t);
    memcpy(request_pipe->pipe_name, buffer + offset, sizeof(char)*MAX_PIPE_NAME);
    offset += (sizeof(char)*MAX_PIPE_NAME);
    memcpy(request_pipe->box_name, buffer + offset, (sizeof(char)*MAX_BOX_NAME));
    offset += (sizeof(char)*MAX_BOX_NAME);
    memcpy(request_pipe->box_password, buffer + offset, sizeof(char)*MAX_PASSWORD);
    return request_pipe;
}

response_manager *reader_stc_response_manager(uint8_t code_pipe,
                                              char buffer[MAX_LINE]) {
    response_manager *response = my_malloc(sizeof(response_manager));
    response->code = code_pipe;
    size_t offset = sizeof(uint8_t);
    memcpy(&response->return_code, buffer + offset, sizeof(int32_t));
    offset += sizeof(int32_t);
    memcpy(response->error_message, buffer + offset, (sizeof(char) * 1024));
    return response;
}

list_manager_response *reader_stc_list_response(uint8_t code_pipe,
                                                char buffer[MAX_LINE]) {
    list_manager_response *list_response =
        my_malloc(sizeof(list_manager_response));
    list_response->code = code_pipe;
    size_t offset = sizeof(uint8_t);
    memcpy(&list_response->last, buffer + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(list_response->box_name, buffer + offset,
           (sizeof(char) * MAX_BOX_NAME));
    offset += (sizeof(char) * MAX_BOX_NAME);
    memcpy(&list_response->box_size, buffer + offset, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    memcpy(&list_response->n_pubs, buffer + offset, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    memcpy(&list_response->n_subs, buffer + offset, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    memcpy(list_response->box_password, buffer + offset, sizeof(char)*MAX_PASSWORD);
    return list_response;
}

list_manager_request *reader_stc_list_request(uint8_t code_pipe,
                                              char buffer[MAX_LINE]) {
    list_manager_request *list_request =
        my_malloc(sizeof(list_manager_request));
    list_request->code = code_pipe;
    memcpy(list_request->pipe_name, buffer + sizeof(uint8_t),
           (sizeof(char) * MAX_PIPE_NAME));
    return list_request;
}

messages_pipe *reader_stc_message(uint8_t code_pipe, char buffer[MAX_LINE]) {
    messages_pipe *message_request = my_malloc(sizeof(messages_pipe));
    message_request->code = code_pipe;
    memcpy(message_request->message, buffer + sizeof(uint8_t),
           (sizeof(char) * 1024));
    return message_request;
}

void *reader_stc(char buffer[MAX_LINE]) {
    void *message;
    uint8_t code_pipe;
    memcpy(&code_pipe, buffer, sizeof(uint8_t));
    switch (code_pipe) {
        case 4:
        case 6:
            message = reader_stc_response_manager(code_pipe, buffer);
            break;
        case 7:
            message = reader_stc_list_request(code_pipe, buffer);
            break;
        case 8:
            message = reader_stc_list_response(code_pipe, buffer);
            break;
        case 9:
        case 10:
            message = reader_stc_message(code_pipe, buffer);
            break;
        default:
            message = reader_stc_request(code_pipe, buffer);
            break;
    }
    return message;
}
