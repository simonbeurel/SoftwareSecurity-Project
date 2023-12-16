#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#define MAX 1024
#define PORT 8080
#define SA struct sockaddr

int sockfd;

int start_server(){
	int connfd, len;
	struct sockaddr_in servaddr, cli;
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

	// Accept the data packet from client and verification
	connfd = accept(sockfd, (SA*)&cli, &len);
	if (connfd < 0) {
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("server accept the client...\n");

	return connfd;
}


void receive_file(int connfd, char *filename) {
    char buff[MAX];
    FILE *fp;
    int n;
	
	printf("Receiving file...%s\n", filename);

    // Open the file for writing
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("Error opening file for writing...\n");
        return;
    }

	while (1) {
		n = recv(sockfd, buff, MAX, 0);
		printf("received %d bytes to server : %s \n", n, buff);
		if (n <= 0){
		break;
		return;
		}
		fprintf(fp, "%s", buff);
		bzero(buff, MAX);
	}
	return;
}


// Function designed for chat between client and server.
void func(int connfd)
{
	char buff[MAX];
	int n;
	// infinite loop for chat
	for (;;) {
		bzero(buff, MAX);

		// read the message from client and copy it in buffer
		read(connfd, buff, sizeof(buff));
		// if msg contains "Exit" then server exit and chat ended.
		if (strncmp("exit", buff, 4) == 0) {
			printf("Server Exit...\n");
			break;
		} else if (strncmp("sectrans -list", buff, 14)==0){
			printf("LIST....\n");
			//Do something......
		} else if (strncmp("sectrans -up", buff, 12) == 0) {
            // Receive file from client
			char *filename = strtok(buff + 13, " \n");
            receive_file(connfd, filename);
        }

	}
}

// Driver function
int main()
{
	int connfd = start_server();

	// Function for chatting between client and server
	func(connfd);

	// After chatting close the socket
	close(sockfd);
}
