#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 1024
#define PORT 8080
#define SA struct sockaddr

void send_file(int sockfd, const char *filename) {
    FILE *fp;
    // Open the file for reading
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error opening file for reading...\n");
        return;
    }
    int n;
    char data[MAX] = {0};

    while(fgets(data, MAX, fp) != NULL) {
        if (send(sockfd, data, sizeof(data), 0) == -1) {
        perror("[-]Error in sending file.");
        exit(1);
        }
        printf("Sent %d bytes to server : %s \n", n, data);
        bzero(data, MAX);
    }

    printf("File sent successfully!\n");
}



void func(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter your command : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        write(sockfd, buff, sizeof(buff));

        // Check if the command is for uploading a file
        if (strncmp("sectrans -up", buff, 12) == 0) {
            // Extract filename from the command
            char *filename = strtok(buff + 13, " \n");
            send_file(sockfd, filename);
        }
    }
}

int main()
{
    int sockfd, connfd;
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
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
        }
    else
        printf("connected to the server..\n");

    // function for chat
    func(sockfd);

    // close the socket
    close(sockfd);
}
