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
#include<netdb.h>
#include"mp1.h"

void error(const char *err)
{
    printf("%s", err);
    exit(-1);
}

int get_socket()
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        error("Error getting socket");
    }
    printf("socket descriptor: %d\n", sfd);
    return sfd;
}

int connect_to_server(int sfd, char *argv[])
{
    struct hostent *server = gethostbyname(argv[1]);
    struct sockaddr_in serv_addr;
    char *str[20];
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    int port = strtol(argv[2], str, 10);
    printf("port %d\n", port);
    serv_addr.sin_port = htons((uint16_t) port);
    int err = connect(sfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if(err < 0) error("ERROR connecting");
    return 0;
}

int get_msg(char *buf)
{
    printf("Enter the String: ");
    fgets(buf, MP1::buf_size, stdin);
    if(buf[0] == 0) error("Invalid input");
    return 0;
}

int send_message(int sfd, char *buf)
{
    printf("Sending message to server: %s", buf);
    ssize_t n = send(sfd, buf, strlen(buf), 0);
    if (n < 0) error("ERROR writing to socket");
    return 0;
}

int listen_for_resp(int sfd, char *buf)
{
    bzero(buf,MP1::buf_size);
    ssize_t n = recv(sfd, buf, MP1::buf_size, 0);
    printf("Got %d chars: %s", (int) n, buf);
    if (n < 0) error("ERROR reading from socket");
    return 0;
}

int main(int argc, char *argv[])
{
    int err = 0;
    char buf [MP1::buf_size];
    int sock_fd = get_socket();
    connect_to_server(sock_fd, argv);
    while(memcmp(buf, "exit\n", sizeof("exit")))
    {
        get_msg(buf);
        send_message(sock_fd, buf);
        listen_for_resp(sock_fd, buf);
    }
    close(sock_fd);
    return 0;
}
