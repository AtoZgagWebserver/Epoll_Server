#include "processepoll.h"

void* epoll_manager(void* arg)
{
    struct EpollManager* epm = (struct EpollManager*)arg;
    struct epoll_event ev[MAX_EVENT_SIZE] ; 
    char buf[4096];
    int ev_size, tmp,size;
    while(1)
    {
        ev_size = epoll_wait(epm->ep,ev, MAX_EVENT_SIZE, -1);//이벤트 발생까지 대기
        if(ev_size < 0)
        {
            perror("epoll_wait Error");
            exit(1);
        }
        for(int i=0; i<ev_size; i++)
        {
            size = recv(ev[i].data.fd, buf, sizeof(buf)-1, 0);
            if(size <=0)
            {
                close(ev[i].data.fd);
                del_fd_from_manager(epm->ep,ev[i].data.fd);
                continue;
            }
            tmp = get_next_worker(epm->work_num,epm->worker);
            struct Work* w = (struct Work*)malloc(sizeof(struct Work));
            w->ep = epm->ep;
            w->msg = strdup(buf);
            w->ns = ev[i].data.fd;
            push(w,epm->worker[tmp].q,&(epm->worker[tmp].cond));
        }
    }
}

struct EpollManager* make_epoll_manager(int numb, int workernum)
{
    struct EpollManager* epm_list = (struct EpollManager*)calloc(sizeof(struct EpollManager),numb);
    for(int i=0; i<numb; i++)
    {
        epm_list[i].ep = epoll_create1(0);
        epm_list[i].work_num = workernum; 
        epm_list[i].worker = make_worker(workernum);
        pthread_create(&epm_list[i].tid, NULL, epoll_manager,(void*)&epm_list[i]);
    }
    
    return epm_list; 
}

void add_fd_to_manager(int ep, int fd)
{
    struct epoll_event ev = {0};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ev);
}

void del_fd_from_manager(int ep,int fd)
{
    epoll_ctl(ep, EPOLL_CTL_DEL, fd, NULL);
}