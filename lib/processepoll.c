#include "processepoll.h"

void* epoll_manager(void* arg)
{
    struct EpollManager* epm = (struct EpollManager*)arg;
    struct epoll_event ev[MAX_EVENT_SIZE] ; 
    int ev_size;
    while(1)
    {
        ev_size = epoll_wait(epm->ep,ev, MAX_EVENT_SIZE, -1);//이벤트 발생까지 대기

        if(ev_size < 0)
        {
            perror("epoll_wait Error");
            exit(1);
        }


    }
}

struct EpollManager* make_epoll_manager(int numb, int workernum)
{
    struct EpollManager* epm_list = (struct EpollManager*)calloc(sizeof(struct EpollManager),numb);
    for(int i=0; i<numb; i++)
    {
        epm_list[i].ep = epoll_create1(0);
        pthread_create(&epm_list[i].tid, NULL, epoll_manager,(void*)&epm_list[i]);
    }
    epm_list[i].worker = make_worker(workernum);
    return epm_list;
}

void add_fd_to_manager(int ep, int fd)
{
    struct epoll_evnet ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    epol_ctl(ep, EPOLL_CTL_ADD, fd, &ev);
}

