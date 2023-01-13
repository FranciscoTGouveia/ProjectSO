#include "writer_stc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>



void writer_stc_request(request* args, char buffer[MAX_LINE]) {
    long unsigned int offset = 0;
    memcpy(buffer, &((request*)args)->code, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(buffer + offset, ((request*)args)->pipe_name, sizeof((request*)args)->pipe_name);
    offset += (sizeof(char)*MAX_PIPE_NAME);
    memcpy(buffer + offset, ((request*)args)->box_name, sizeof((request*)args)->box_name);
    printf("dentro do writer este e o box %s\n",((request*)args)->box_name);
    char teste[MAX_BOX_NAME];
    memcpy(teste, buffer + offset, (sizeof(teste)));
    printf("dentro do writer tentativa de conversao %s\n", teste);
    printf("len dentro do writer %ld\n", strlen(buffer));
}



void writer_stc_response_manager(response_manager* args, char buffer[MAX_LINE]) {
    long unsigned int offset = 0;
    memcpy(buffer, &((response_manager*)args)->code, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(buffer + offset, &((response_manager*)args)->return_code, sizeof(int32_t));
    offset += sizeof(int32_t);
    memcpy(buffer + offset, ((response_manager*)args)->error_message,
    sizeof((response_manager*)args)->error_message);
}



void writer_stc_list_request(list_manager_request* args, char buffer[MAX_LINE]) {
    memcpy(buffer,&((list_manager_request*)args)->code, sizeof(uint8_t));
    memcpy(buffer + sizeof(uint8_t), ((list_manager_request*)args)->pipe_name,
    sizeof((list_manager_request*)args)->pipe_name);
}



void writer_stc_list_response(list_manager_response* args, char buffer[MAX_LINE]) {
    long unsigned int offset = 0;
    memcpy(buffer, &((list_manager_response*)args)->code, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(buffer + offset, &((list_manager_response*)args)->last, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(buffer + offset, ((list_manager_response*)args)->box_name, 
    sizeof(((list_manager_response*)args)->box_name));
    offset += (sizeof(char)*MAX_BOX_NAME);
    memcpy(buffer + offset, &((list_manager_response*)args)->box_size,
    sizeof(uint64_t));
    offset += sizeof(uint64_t);
    memcpy(buffer + offset, &((list_manager_response*)args)->n_pubs, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    memcpy(buffer + offset, &((list_manager_response*)args)->n_subs, sizeof(uint64_t));
}


void writer_stc_message(messages_pipe* args, char buffer[MAX_LINE]) {
   memcpy(buffer, &((messages_pipe*)args)->code, sizeof(uint8_t));
   uint8_t teste1;
   memcpy(&teste1, buffer, sizeof(uint8_t));
   printf("DENTRO DO WRITE CODE %d \n", teste1);
   memcpy(buffer + sizeof(uint8_t), 
   ((messages_pipe*)args)->message, sizeof(((messages_pipe*)args)->message));
   printf("etsa foi a mensagem q recebeu %s \n", args->message);
   char teste[1024];
   memcpy(teste, buffer + sizeof(uint8_t), sizeof(teste) );
   printf("esta foi a mensagem traduzida %s \n", teste);
}

void writer_stc(void* args, uint8_t code_pipe, char buffer[MAX_LINE]) {
    memset(buffer, 0, MAX_LINE);
    switch (code_pipe) {
        case 4:
            writer_stc_response_manager(args, buffer);
            break;
        case 6:
            writer_stc_response_manager(args, buffer);
            break;
        case 7:
            writer_stc_list_request(args, buffer);
            break;
        case 8:
            writer_stc_list_response(args, buffer);
            break;
        case 9:
            writer_stc_message(args, buffer);
            break;
        case 10:
            writer_stc_message(args, buffer);
            break;
        default:
            writer_stc_request(args,buffer);
            break;
    }
    
}