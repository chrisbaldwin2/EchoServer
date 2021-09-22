/* run_server.cpp
 * 
 * Original Author: Chris Baldwin
 * Partner:         Kaushal Prudhvi
 * Date:            9/20/2021
 * Course:          ECEN 602 Fall 2021
 * Assignmnet:      MP1
 * Professor:       Dr. Narasimha Annapareddy
 * TA:              Rishabh Singla
 * 
 * History Table
 * ===========================================================
 * Date       :: Author        :: Change
 * -----------++---------------++-----------------------------
 * 9/20/2021  :: Chris Baldwin :: Created run_server.cpp file
 * -----------++---------------++-----------------------------
 * 9/21/2021  :: Chris Baldwin :: Added errno handling
 * -----------++---------------++-----------------------------
 * 9/21/2021  :: Chris Baldwin :: Added function header 
 *            ::               :: comments and history table
 * -----------++---------------++-----------------------------
 * 9/21/2021  :: Chris Baldwin :: Added better newline handling 
 *            ::               :: and buffer underflow in r/w
 * -----------++---------------++-----------------------------
 */

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

/* error
 *
 * Prints the reason for the error and exits the program with code -1
 * 
 * @param err The string describing why the error has occured
 * @return none
 */
void error(const char *err)
{
    printf("%s", err);
    exit(MP1::ERROR);
}

/* bind_socket
 *
 * Acquires a socket, binds it, and begins listening
 * 
 * @param address The struct containing the addresses to accept (INADDR_ANY), 
 *                port to bind, and the family (IPv4)
 * @return sfd The socket file descriptor
 */
int bind_socket(struct sockaddr_in *address)
{
    int sfd, err;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0) error("Error acquiring socket\n");
    // printf("socket descriptor: %d\n", sfd);
    err = bind(sfd, (struct sockaddr *) address, sizeof(struct sockaddr));
    if(sfd < 0) error("Error binding socket\n");
    // printf("bind: %d\n", err);
    err = listen(sfd, MP1::list_queue_size);
    if(err < 0) error("Error listening on socket\n");
    // printf("listen: %d\n", err);
    return sfd;
}

/* read_socket
 *
 * Reads from the socket file descriptor to th buffer. If 
 * the write returns less than the size of the buffer, this 
 * wrapper will attempt to write the rest of the buffer.
 * 
 * @param sfd The open socket file descriptor
 * @param buf The buffer to be written to the socket
 * @param index The index of the buffer to start writting
 * @return size The size of the buffer written to the sfd 
 */
int read_socket(int sfd, char *buf, int index)
{
    int size;
    read_l:
      size = read(sfd, buf + index, MP1::buf_size - index);
      if(size < 0 && errno == EINTR) goto read_l;
    if(size < 0) error("Error receiving packet\n");
    if(size == 0) return 0;
    if(size + index < MP1::buf_size) read_socket(sfd, buf, size + index);
    return size + index;
}

/* write_socket
 *
 * Writes the buffer to the socket file descriptor. If the write
 * returns less than the size of the buffer, this wrapper will 
 * attempt to write the rest of the buffer.
 * 
 * @param sfd The open socket file descriptor
 * @param buf The buffer to be written to the socket
 * @param index The index of the buffer to start writting
 * @return size The size of the buffer written to the sfd 
 */
int write_socket(int sfd, char *buf, int index)
{
    int size;
    write_l:
      size = write(sfd, buf + index, MP1::buf_size - index);
      if(size < 0 && errno == EINTR) goto write_l;
    if(size < 0) error("Error sending echo packet\n");
    if(size + index < MP1::buf_size) write_socket(sfd, buf, index + size);
    return size + index;
}

/* listen_on_socket
 *
 * Listens for a new connenction, accepts it, forks, and echos any packets which are sent.
 * On sending a blank buffer (EOF), the connection is closed and the child process ended.
 * 
 * @param sfd The open socket file descriptor
 * @param port The port number passed in from command line
 * @return none
 */
void listen_on_socket(int sfd, int port)
{
    printf("Listening on sock %d port %d\n", sfd, port);
    sockaddr_in cli_addr;
    bzero((char *) &cli_addr, sizeof(sockaddr_in));
    socklen_t clilen = sizeof(sockaddr);
    int newsockfd, pid;
    signal(SIGCHLD, SIG_IGN);
    while(1)
    {
        newsockfd = accept(sfd, (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0) error("Error accepting connection\n");
        printf("Opening client %d\n", cli_addr.sin_port);
        pid = fork();
        if(pid < 0) error("Error forking proccess\n");
        if(pid == 0){
            // Child Process
            char buf[MP1::buf_size];
            char ip_str[INET_ADDRSTRLEN];
            ssize_t size;
            // Stores the string of the client ip address into ip_str
            inet_ntop(AF_INET, &(cli_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
            while(1)
            {
                // Zero the buffer to prevent reading stale values
                bzero(buf, MP1::buf_size);
                size = read_socket(newsockfd, buf, 0);
                if(size == 0) break;
                printf("Received @ %s::%d size %d: %s", ip_str, cli_addr.sin_port, (int) size, buf);
                if(!strstr(buf, "\n")) printf("\n");
                printf("Echoing: %s", buf);
                if(!strstr(buf, "\n")) printf("\n");
                size = write_socket(newsockfd, buf, 0); 
            }
            // After receiving EOF, Close the socket and end the child process
            printf("Closing client %d\n", cli_addr.sin_port);
            close(newsockfd);
            exit(MP1::GOOD);
        }
        // Parent Process
        close(newsockfd);
    }
}

int main(int argc, char *argv[])
{
    int port, sfd;
    char *port_str[20];
    // Handle issues with port number
    if(argc != 2) error("Call must have 1 argument: PORT_NUM\n");
    port = strtol(argv[1], port_str, 10);
    if(port <= 1024) error("Error port cannot be less than 1024\n");
    // Assign socket struct variables
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) port);
    // Bind the socket
    sfd = bind_socket(&serv_addr);
    // Listen to socket ( infinite loop )
    listen_on_socket(sfd, port);
    return MP1::GOOD;
}