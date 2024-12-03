#include "threadfunc.h"
#include "processepoll.h"
#include "httpfunc.h"

int handle_client(struct Work* data)
{
    struct HTTPRequest http_request = {0};
    parse_http_request(data->msg, &http_request);

    if(strcmp(http_request.method, "GET") == 0)
    {
        if(strcmp(http_request.path, "/quiz") == 0)
        {
            send_quiz(data->ns);
        }
        else
        {
            char file_path[512];
            snprintf(
                file_path,
                sizeof(file_path),
                "./rsc/html/%s",
                http_request.path[0] == '/' ? http_request.path + 1 : http_request.path
            );
            send_file_content(data->ns, file_path);
        }
    }
    else
    {
        const char * not_found_response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n404 Not Found";
        send(data->ns, not_found_response, strlen(not_found_response), 0);
    }

    close(data->ns);
    del_fd_from_manager(data->ep,data->ns);

    return 0;
}

void* worker(void* arg) // worker number
{
    struct ThrInfo* inf = (struct ThrInfo*)arg;

    while(1)
    {
        struct Work* w = pop(inf->q);
        if(w == NULL)
        {
            //printf("Stop Thread %d\n", inf->number);
            pthread_mutex_lock(&(inf->mut));
            pthread_cond_wait(&(inf->cond), &(inf->mut));
            //printf("Restart Thread %d\n", inf->number);
            pthread_mutex_unlock(&(inf->mut));
            continue;
        }
        handle_client(w);
    }
}

struct ThrInfo* make_worker(int work_num)
{
    struct ThrInfo* thrinflist = (struct ThrInfo*)malloc(sizeof(struct ThrInfo)*work_num);
    int tmp;
    
    for(int i=0; i<work_num; i++)
    {
        thrinflist[i].number = i;
        thrinflist[i].q = new_queue();
        tmp = pthread_cond_init(&(thrinflist[i].cond), NULL);
        tmp = pthread_mutex_init(&(thrinflist[i].mut), NULL);

        pthread_create(&thrinflist[i].tid,NULL,worker,(void*)&thrinflist[i]);
    }

    return thrinflist;
}

int get_next_worker(int work_num, struct ThrInfo* thrinflist)
{
    int least = 0;
    for(int i=1; i<work_num; i++)
    {
        if(size(thrinflist[i].q) > size(thrinflist[least].q))
        {
            least = i;
        }
    }
    return least;
}

struct Queue* new_queue()
{
    // make new queue
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));

    // initialize
    q->items = (struct Work*)malloc(sizeof(struct Work)*5000);
    q->maxsize = 5000;
    q->front = 0;
    q->rear = 0;

    //return
    return q;
}

int empty(struct Queue* q)
{
    return q->front==q->rear;
}

int size(struct Queue* q)
{
    return q->rear-q->front;   
}

struct Work* pop(struct Queue* q)
{
    //if queue is empty than return NULL;
    if(empty(q))return NULL;

    struct Work* res = &(q->items[q->front]);
    q->front = (q->front+1)%q->maxsize;

    return res;
}

void push(struct Work* w,struct Queue* q,pthread_cond_t* cond)
{
    q->items[q->rear] = *w;
    q->rear = (q->rear + 1)%q->maxsize;
    if(q->rear == q->front) // size reallocation 
    {
        if(realloc(q->items,sizeof(q->items)*2)==NULL)
        {
            perror("Realloc");
            exit(1);
        }
        // move item to new space
        if(q->front > q->rear)
        {
            for(int i=0; i<q->rear; i++)
            {
                q->items[q->maxsize+i] = q->items[i];
            }
        }
        q->rear = q->maxsize + q->rear;
        q->maxsize*=2;
    }
    //printf("Send signal\n");
    pthread_cond_signal(cond);
    //printf("Sending signal done\n");
}