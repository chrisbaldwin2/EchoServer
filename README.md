# EchoServer
## Authors 
- Chris Baldwin
- Kaushal Prudhvi

## Roles
Chris Baldwin
* Source Code
* Makefile
* README

Kaushal Prudhvi
* Code Review
* Testcases
* Report

__To Install__
1. Clone the repo
2. Open a terminal (server) in the repository folder
3. run `make` to make the project
4. run `hostname -I` to get your local ip address
5. run `./run_server <PORT_NUM>`
6. Open a terminal (client) in the repository folder
7. run `./run_client <IP_ADDR> <PORT_NUM>` (Note that 
   the PORT_NUM should match the server port number)

__Running The Server__
* The buffer size is 80 Bytes
* The client can type Cntrl-D or Cntrl-C to end their session
