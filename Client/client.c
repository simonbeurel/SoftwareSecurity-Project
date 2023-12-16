#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SIZE 1024

int sockfd;

void connection(){
    char *ip = "127.0.0.1";
	int port = 8080;
	int e;

    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("[-]Error in socket");
		exit(1);
	}
	printf("[+]Server socket created successfully.\n");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = port;
	server_addr.sin_addr.s_addr = inet_addr(ip);

    e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(e == -1){
        perror("[-]Error in socket");
        exit(1);
    }
    printf("[+]Connected to Server.\n");

}


void send_file(int sockfd){
    FILE *fp;
    char *filename = "test.txt";

    fp = fopen(filename, "rb");
    if(fp == NULL){
        perror("[-]Error in reading file.");
        exit(1);
    }

    char data[SIZE] = {0};
    while(fgets(data, SIZE, fp) != NULL){
        if(send(sockfd, data, sizeof(data), 0) == -1){
            perror("[-]Error in sending file.");
            exit(1);
        }
        bzero(data, SIZE);
    }

}

int main(){

    connection();

    /*while(1){

    }*/

    send_file(sockfd);
    

}