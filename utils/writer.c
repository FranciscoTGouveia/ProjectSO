#include "writer.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>



void writer_request(request* args, char buffer[MAX_LINE]) {
    char pipe[] = "|";
    char code_str[100];
    sprintf(code_str, "%u", args->code);
    strcat(buffer, code_str);
    strcat(buffer, pipe);
    strcat(buffer, args->pipe_name);
    strcat(buffer, pipe);
    strcat(buffer, args->box_name);
}

void writer_response_manager(response_manager* args, char buffer[MAX_LINE]) {
    char pipe[] = "|";
    char code_str[100];
    char return_str[100];
    sprintf(code_str, "%u", args->code);
    sprintf(return_str, "%d", args->return_code);
    strcat(buffer, code_str);
    strcat(buffer, pipe);
    strcat(buffer, return_str);
    strcat(buffer, pipe);
    strcat(buffer, args->error_message);
}

void writer_list_request(list_manager_request* args, char buffer[MAX_LINE]) {
    char pipe[] = "|";
    char code_str[100];
    sprintf(code_str, "%u", args->code);
    strcat(buffer, code_str);
    strcat(buffer, pipe);
    strcat(buffer, args->pipe_name);
}

void writer_list_response(list_manager_response* args, char buffer[MAX_LINE]) {
    char pipe[] = "|";
    //int len_box = (int)(log10((double)args->box_size) + 1);
    //int len_sub = (int)(log10((double)args->n_subs) + 1);
    int len_box = 1000;
    int len_sub = 1000;
    char code_str[100], last_str[100], size_str[len_box]; 
    char pub_str[2], sub_str[len_sub];
    sprintf(code_str, "%u", args->code);
    sprintf(last_str, "%u", args->last);
    sprintf(size_str, "%lu", args->box_size);
    sprintf(pub_str, "%lu", args->n_pubs);
    sprintf(sub_str, "%lu", args->n_subs);
    strcat(buffer, code_str);
    strcat(buffer, pipe);
    strcat(buffer, last_str);
    strcat(buffer, pipe);
    strcat(buffer, args->box_name);
    strcat(buffer, pipe);
    strcat(buffer, size_str);
    strcat(buffer, pipe);
    strcat(buffer, pub_str);
    strcat(buffer, pipe);
    strcat(buffer, sub_str);
}

void writer_message(messages_pipe* args, char buffer[MAX_LINE]) {
    char pipe[] = "|";
    char code_str[100];
    sprintf(code_str, "%u", args->code);
    strcat(buffer, code_str);
    strcat(buffer,pipe);
    strcat(buffer,args->message);
}

void writer(void* args, uint8_t code_pipe, char buffer[MAX_LINE]) {
    switch (code_pipe) {
        case 4:
            writer_response_manager(args, buffer);
            break;
        case 6:
            writer_response_manager(args, buffer);
            break;
        case 7:
            writer_list_request(args, buffer);
            break;
        case 8:
            writer_list_response(args, buffer);
            break;
        case 9:
            writer_message(args, buffer);
            break;
        case 10:
            writer_message(args, buffer);
            break;
        default:
            writer_request(args,buffer);
            break;
    }
}

