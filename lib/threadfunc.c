#include "threadfunc.h"
#include "processepoll.h"
#include "httpfunc.h"

int handle_client(struct Work* w)
{
    int ns = w->ns;
    char buf[HTTP_BUF_MAX_SIZE]; // 수신 버퍼
    char data[HTTP_BUF_MAX_SIZE]; // 요청 데이터를 저장할 배열
    ssize_t n;
    int data_len = 0;

    // 클라이언트로부터 HTTP 요청을 수신
    n = recv(ns, data, HTTP_BUF_MAX_SIZE - 1, 0);
    data_len=strlen(data);

    if(n>0){
        data[data_len] = '\0';

        struct HTTPRequest http_request;
        memset(&http_request, 0, sizeof(struct HTTPRequest));

        parse_http_request(data, &http_request);

        printf("Method: %s\n", http_request.method);
        printf("Path: %s\n", http_request.path);
        if (http_request.content_length > 0) {
             printf("Content-Length: %d\n", http_request.content_length);
        }
        if (strlen(http_request.body) > 0) {
             printf("Body:\n%s\n", http_request.body);
        }

        //요청에 따라 어떻게 처리할지
        if (strcmp(http_request.method, "GET") == 0) 
        { //GET 요청의 경우
            if(strcmp(http_request.path,"/quiz")==0){
                send_quiz(ns);
            }
            else{
                printf("get -> html리턴\n");
                char file_path[512];
                snprintf(file_path, sizeof(file_path), "./rsc/html/%s", http_request.path[0] == '/' ? http_request.path + 1 : http_request.path);
                send_file_content(ns, file_path);
            }
        }
        if (strcmp(http_request.method, "POST") == 0) 
        {   
            if(1){
                printf("not found\n");
                const char *not_found_response = 
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 13\r\n"
                    "\r\n"
                    "404 Not Found";
                send(ns, not_found_response, strlen(not_found_response), 0);
            }
        }
    }
    
    close(ns);
    //free(w);

    return 0;
}

void* worker(void* arg) // worker number
{
    struct ThrInfo* inf = (struct ThrInfo*)arg;

    while(1) // thread 개수는 미정 차차 맞춰갈 예정임.
    {
        struct Work* w = pop(inf->q);
        if(w == NULL)
        {
            printf("Stop Thread %d\n", inf->number);
            pthread_mutex_lock(&(inf->mut));
            pthread_cond_wait(&(inf->cond), &(inf->mut));
            printf("Restart Thread %d\n", inf->number);
            pthread_mutex_unlock(&(inf->mut));
            continue;
        }
        handle_client(w);
    }
}

struct ThrInfo* make_worker(int work_num)
{
    pthread_t* tid_list = (pthread_t*) malloc(sizeof(pthread_t)*work_num);
    struct ThrInfo* thrinflist = (struct ThrInfo*)malloc(sizeof(struct ThrInfo)*work_num);
    int tmp;

    for(int i=0; i<work_num; i++)
    {
        thrinflist[i].number = i;
        thrinflist[i].q = new_queue();
        thrinflist[i].ep_ins = epoll_create1(0);
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
    q->items = (struct Work*)malloc(sizeof(struct Work));
    q->maxsize = 1;
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
    printf("Send signal\n");
    pthread_cond_signal(cond);
    printf("Sending signal done\n");
}