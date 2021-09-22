/* run_client.cpp
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
 * 9/20/2021  :: Chris Baldwin :: Created run_client.cpp file
 * -----------++---------------++-----------------------------
 * 9/21/2021  :: Chris Baldwin :: Added errno handling
 * -----------++---------------++-----------------------------
 * 9/21/2021  :: Chris Baldwin :: Added function header 
 *            ::               :: comments and history table
 * -----------++---------------++-----------------------------
 * 9/21/2021  :: Chris Baldwin :: Added better newline handling 
 *            ::               :: and buffer underflow in r/w
 * -----------++---------------++-----------------------------
 * 9/21/2021  :: Chris Baldwin :: Added EOF handling
 * -----------++---------------++-----------------------------
 * 9/22/2021  :: Chris Baldwin :: Cleaned up printf statements
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
 * Acquires a socket
 * 
 * @param none
 * @return sfd The socket file descriptor
 */
int get_socket()
{
    int sfd;
    // Attempt to acquire a TCP IPv4 socket
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0) error("Error acquiring socket\n");
    // printf("socket descriptor: %d\n", sfd);
    return sfd;
}

/* connect_to_server
 *
 * Connects to a server specified in argv where argv[1] is the ip 
 * and argv[2] is the port number
 * 
 * @param sfd The open socket file descriptor
 * @param argv The arguments passed in from the command line
 * @return none
 */
void connect_to_server(int sfd, char *argv[])
{
    struct sockaddr_in serv_addr;
    char *str[20];
    int port, err;
    // Zero out the memory @ serv_addr
    bzero((char *) &serv_addr, sizeof(serv_addr));
    // Set values for sockaddr_in struct
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    port = strtol(argv[2], str, 10);
    printf("Connecting to %s::%d\n", argv[1], port);
    serv_addr.sin_port = htons((uint16_t) port);
    // Try to connect to the ip and port number specified
    err = connect(sfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if(err < 0) error("Error connecting to server\n");
}

/* readline
 *
 * Reads from stdin and writes it to buf
 * 
 * @param[out] buf The buffer to write to 
 * @return none
 */
int readline(char *buf)
{
    printf("Enter the String: ");
    // Read from stdin at most the size of the buffer
    if(!fgets(buf, MP1::buf_size, stdin)) 
    {
        printf("<EOF>\n");
        return MP1::ERROR;
    }
    if(!strstr(buf, "\n")) printf("\n");
    return MP1::GOOD;
}

/* writen
 *
 * Writes the buffer passed in to the connected server
 * 
 * @param sfd The open socket file descriptor
 * @param buf The buffer to be written to the server
 * @return none
 */
void writen(int sfd, char *buf, int index)
{
    ssize_t size;
    printf("Sending message: %s", buf);
    if(!strstr(buf, "\n")) printf("\n");
    // Attempt to send the packet to the server & retry on EINTR
    write_l:
      size = write(sfd, buf + index, MP1::buf_size - index);
      if(size < 0 && errno == EINTR) goto write_l;
    if(size < 0) error("Error writing to socket\n");
    if(size + index < MP1::buf_size) writen(sfd, buf, index + size);
}

int readn(int sfd, char *buf, int index)
{
    int size;
    read_l:
      size = read(sfd, buf + index, MP1::buf_size - index);
      if(size < 0 && errno == EINTR) goto read_l;
    if(size < 0) error("Error reading from socket\n");
    if(size + index < MP1::buf_size) readn(sfd, buf, index + size);
    return size;
}

/* listen_for_resp
 *
 * Waits for the echo from the server and prints it to
 * stdout
 * 
 * @param[in]  sfd The open socket file descriptor
 * @param[out] buf The buffer to store the echo resp in
 * @return none
 */
void listen_for_resp(int sfd, char *buf)
{
    ssize_t size;
    bzero(buf,MP1::buf_size);
    // Attempt to receive the packet from the server & retry on EINTR
    size = readn(sfd, buf, 0);
    printf("Got %d chars: %s", (int) size, buf);
    if(!strstr(buf, "\n")) printf("\n");
}

int main(int argc, char *argv[])
{
    char buf [MP1::buf_size];
    int sock_fd;
    if(argc != 3) error("Call must have 2 arguments: IPv4_ADDRESS PORT_NUM\n");
    sock_fd = get_socket();
    connect_to_server(sock_fd, argv);
    while(1)
    {
        if(readline(buf) == MP1::ERROR) break;
        writen(sock_fd, buf, 0);
        listen_for_resp(sock_fd, buf);
    }
    // Close the socket ( sending EOF in the proccess )
    printf("~~Terminating Session~~\n");
    close(sock_fd);
    return MP1::GOOD;
}
