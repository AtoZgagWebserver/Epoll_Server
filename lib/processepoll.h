#include "headerlist.h"
#include "threadfunc.h"
#ifndef EPOLLSTRUCT
#define EPOLLSTRUCT

#define MAX_EVENT_SIZE 4096

struct EpollManager
{
    int ep, size;
    pthread_t tid;
    struct ThrInfo* worker;
}

#endif

struct EpollManager* make_epoll_manager(int numb,int workernum);
int add_fd_to_manager(int ep, int fd);