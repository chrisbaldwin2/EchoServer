#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<strings.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>
#include"mp1.h"
#include<signal.h>

void error(const char *err)
{
    printf("%s", err);
    exit(-1);
}

int bind_socket(struct sockaddr_in *address)
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) error("Error getting socket");
    // printf("socket descriptor: %d\n", sfd);
    int err = bind(sfd ,(struct sockaddr *)address,sizeof(struct sockaddr));
    if (sfd < 0) error("Error getting socket");
    // printf("bind: %d\n", err);
    err = listen(sfd, MP1::list_queue_size);
    if(err < 0) error("Error on listen");
    // Handle socket with interupts
    // while(err == EINTR) err = listen(sfd, MP1::list_queue_size);
    // printf("listen: %d\n", err);
    return sfd;
}

int listen_on_socket(struct sockaddr_in *cli_addr, int sfd, int port)
{
    printf("Listening on sock %d port %d\n", sfd, port);
    socklen_t clilen = sizeof(sockaddr);
    char buf[MP1::buf_size];
    while(1)
    {
        int newsockfd = accept(sfd, (struct sockaddr *) cli_addr, &clilen);
        if (newsockfd < 0) error("Accept");
        printf("Opening client %d\n", cli_addr->sin_port);
        int pid = fork();
        if(pid < 0) error("Error listening on socket");
        if(pid == 0){
            // Child Process
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(cli_addr->sin_addr), ip_str, INET_ADDRSTRLEN);
            while(1)
            {
                bzero(buf, MP1::buf_size);
                ssize_t size;
                read_l:
                  size = recv(newsockfd, buf, MP1::buf_size, 0);
                  if(size < 0 && errno == EINTR) goto read_l;
                if(size < 0) error("Receive");
                if(size == 0) break;
                printf("Received @ %s::%d size %d: %s", ip_str, cli_addr->sin_port, (int) size, buf);
                printf("Echoing: %s", buf);
                write_l:
                  size = send(newsockfd, buf, MP1::buf_size, 0);
                  if(size < 0 && errno == EINTR) goto write_l;
                if(size < 0) error("Send"); 
            }
            printf("Closing client %d\n", cli_addr->sin_port);
            close(newsockfd);
            exit(0);
        }
        // Parent Process
        signal(SIGCHLD,SIG_IGN);
        close(newsockfd);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    sockaddr_in serv_addr, cli_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    char *str[20];
    int port = strtol(argv[1], str, 10);
    if(port < 1024) port = MP1::port_num;
    serv_addr.sin_port = htons((uint16_t) port);
    int sfd = bind_socket(&serv_addr);
    listen_on_socket(&cli_addr, sfd, port);
    return 0;
}