#include "lib/headerlist.h"
#include "lib/readdata.h"
#include "lib/processepoll.h"

struct QuestionList *question;

int main(int argc, char* argv[])
{
    //default set
    if(argc != 4)//./server portnum managernum workernum
    {
        perror("Argument num is wrong");
        exit(1);
    }
    int portnum = atoi(argv[1]), mannum = atoi(argv[2]), worknum = atoi(argv[3]);
    struct sockaddr_in sin, cli;
    int sd, ns, clientlen = sizeof(cli);
    
    // create socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1)
    {
        perror("socket");
        exit(1);
    }

    // port recycle
    int optval = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // create socket struct
    memset((char *)&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(portnum);
    sin.sin_addr.s_addr = inet_addr("0.0.0.0");

    // bind
    if (bind(sd, (struct sockaddr *)&sin, sizeof(sin)))
    {
        perror("bind error");
        exit(1);
    }
    if(listen(sd, 50000)) // second parameter is BACKLOG That is max size of queue
    {
        perror("listen error");
        exit(1);
    }
    // prepare for clients
    printf("Prepare to accept client\n");

    printf("    Create epoll manager.\n");

    struct EpollManager* epm_list = make_epoll_manager(mannum, worknum/mannum); 

    printf("    Create epoll manager Done.\n")
    
    printf("Ready for accept client\n");

    // accept the clients
    int next_worker_num;
    while(1)
    {
        next_worker_num = get_next_worker(worknum, worker);
        int* ns = (int*)malloc(sizeof(int));

		if((ns=accept(sd,(struct sockaddr*)&cli,&clientlen))==-1)
        {
			perror("accept");
			exit(1);
		}
        printf("accept\n");
        printf("push work to worker[%d]\n", next_worker_num);
        add_fd_to_manager(epm_list[cnt].ep, ns);
        cnt=(cnt+1)%mannum;
        printf("push done\n");
    }
	close(sd);
}