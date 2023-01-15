#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

void* my_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        printf("Falha na alocação de memória.\n");
        // codigo para limpar todas as merdas
        exit(1);
    }
    return ptr;
}