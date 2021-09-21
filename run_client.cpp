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
// #include<netdb.h>
#include"mp1.h"

void error(const char *err)
{
    printf("%s", err);
    exit(-1);
}

int get_socket()
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) error("Error getting socket");
    printf("socket descriptor: %d\n", sfd);
    return sfd;
}

int connect_to_server(int sfd, char *argv[])
{
    // struct hostent *server = gethostbyname(argv[1]);
    struct sockaddr_in serv_addr;
    char *str[20];
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // bcopy((char *)server->h_addr, 
        //  (char *)&serv_addr.sin_addr.s_addr,
        //  server->h_length);
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    int port = strtol(argv[2], str, 10);
    printf("port %d\n", port);
    serv_addr.sin_port = htons((uint16_t) port);
    int err = connect(sfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if(err < 0) error("ERROR connecting");
    return 0;
}

int readline(char *buf)
{
    printf("Enter the String: ");
    fgets(buf, MP1::buf_size, stdin);
    if(buf[0] == 0) error("Invalid input");
    return 0;
}

int writen(int sfd, char *buf)
{
    printf("Sending message to server: %s", buf);
    ssize_t size;
    write_l:
      size = send(sfd, buf, strlen(buf), 0);
      if(size < 0 && errno == EINTR) goto write_l;
    if (size < 0) error("ERROR writing to socket");
    return 0;
}

int listen_for_resp(int sfd, char *buf)
{
    bzero(buf,MP1::buf_size);
    ssize_t size;
    read_l:
      size = recv(sfd, buf, MP1::buf_size, 0);
      if(size < 0 && errno == EINTR) goto read_l;
    if (size < 0) error("ERROR reading from socket");
    printf("Got %d chars: %s", (int) size, buf);
    return 0;
}

int main(int argc, char *argv[])
{
    char buf [MP1::buf_size];
    int sock_fd = get_socket();
    connect_to_server(sock_fd, argv);
    while(1)
    {
        readline(buf);
        // printf("comp:%d\n", memcmp(buf, "exit\n", sizeof("exit")));
        if(!memcmp(buf, "EOF\n", sizeof("EOF"))) break;
        writen(sock_fd, buf);
        listen_for_resp(sock_fd, buf);
    }
    bzero(buf,MP1::buf_size);
    writen(sock_fd, buf);
    printf("\n");
    close(sock_fd);
    return 0;
}
