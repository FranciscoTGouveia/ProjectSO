#ifndef __TASK_H__
#define __TASK_H__

typedef struct {
    void (*function)(void*);
    void* request; //have to free
} task;

#endif