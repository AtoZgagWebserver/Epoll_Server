#include "headerlist.h"

#ifndef THREADSTRUCT
#define THREADSTRUCT

struct ThrInfo
{
    int number;
    pthread_t tid;
    struct Queue* q;
    pthread_mutex_t mut;
    pthread_cond_t cond;
};

struct Work
{
    int ns;
    int ep;
    char* msg;
};

struct Queue
{
    struct Work* items;
    int front,rear;
    int maxsize;
};

#endif

//queue function
struct Queue* new_queue();
int empty(struct Queue* q);
int size(struct Queue* q);
struct Work* pop(struct Queue* q);
void push(struct Work* w,struct Queue* q,pthread_cond_t* cond);

//thread function
struct ThrInfo* make_worker(int work_num);
int get_next_worker(int work_num, struct ThrInfo* thrinflist);