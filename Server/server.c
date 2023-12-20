#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include "client.h"
#include "server.h"

int main(){
	startserver(8080);
    printf("Serveur activé...\n");

    while(1){

        char rcv_msg[1024];
        getmsg(rcv_msg);

        printf("Message reçu :%s\n",rcv_msg);

        if(strncmp(rcv_msg,"exit", 4) == 0){
            break;
        }
        else if (strncmp(rcv_msg,"sectrans -list", 14) == 0) {
            printf("Liste des fichiers demandée...\n");
            char msg[1024] = "";
            strcat(msg, "Liste des fichiers disponibles :\n");
            DIR *dir;
            struct dirent *entry;
            dir = opendir("./database");
            if (dir == NULL) {
                perror("Erreur lors de l'ouverture du répertoire");
                exit(EXIT_FAILURE);
            }
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_REG) {
                    strcat(msg, entry->d_name);
                    strcat(msg, "\n");
                }
            }
            closedir(dir);
            printf("%s\n",msg);
            sndmsg(msg,8081);
        }
    }
    stopserver();
    printf("Fin du server....\n");
}