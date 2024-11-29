#include "lib/headerlist.h"
#include "lib/threadfunc.h"

int main(int argc, char* argv[])
{
    //default set
    if(argc < 3)
    {
        perror("Few argument");
    }
    int portnum = atoi(argv[1]), worknum = atoi(argv[2]);
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

    printf("    Create workers\n");
    
    struct ThrInfo* worker = make_worker(worknum); 
    
    printf("    Create workers Done.\n");

    printf("Ready for accept client\n");

    // accept the clients
    int next_worker_num;
    while(1)
    {
        next_worker_num = get_next_worker(worknum, worker);
        struct Work *work = (struct Work*)malloc(sizeof(struct Work));
        
		if((work->ns=accept(sd,(struct sockaddr*)&cli,&clientlen))==-1)
        {
			perror("accept");
			exit(1);
		}
        printf("accept\n");
        printf("push work to worker[%d]\n", next_worker_num);
        push(work, worker[next_worker_num].q);
        printf("push done\n");
    }
	close(sd);
}