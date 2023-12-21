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
        }else if (strncmp(rcv_msg,"sectrans -up", 12) == 0) {
            printf("Reception dun fichier\n");
            char *copy = strdup(rcv_msg);
            char *saveptr;
            char *tokenized = strtok_r(copy, " ", &saveptr);
            for (int i = 0; i < 2 && tokenized != NULL; ++i) {
                tokenized = strtok_r(NULL, " ", &saveptr);
            }
            if (tokenized != NULL) {
                char msg[1024];
                getmsg(msg);
                if(strncmp(msg,"ERROR",5) == 0){
                    printf("Erreur lors de la reception du fichier\n");
                }else{
                    //create the file on the database folder
                    const char *directory = "./database/";
                    char filepath[1024];
                    strcpy(filepath, directory);
                    strcat(filepath, tokenized);
                    FILE *file = fopen(filepath, "w");
                    //write all msg in the new file
                    fwrite(msg, 1, strlen(msg), file);
                    fclose(file);
                    printf("Fichier bien reçu\n");

                    char msg2[1024] = "Fichier bien reçu";
                    sndmsg(msg2, 8081);
                }
            }
        }else if (strncmp(rcv_msg,"sectrans -down", 14) == 0){
            char *copy = strdup(rcv_msg);
            char *saveptr;
            char *tokenized = strtok_r(copy, " ", &saveptr);
            for (int i = 0; i < 2 && tokenized != NULL; ++i) {
                tokenized = strtok_r(NULL, " ", &saveptr);
            }
            if (tokenized != NULL) {
                char msg[1024];
                //open the file "tokenized"
                const char *directory = "./database/";
                char filepath[1024];
                strcpy(filepath, directory);
                strcat(filepath, tokenized);
                FILE *file = fopen(filepath, "r");
                if (file == NULL) {
                    printf("Erreur lors de l'ouverture du fichier pour l'envoi\n");
                    char msg2[1024] = "Erreur : Le fichier demandé n'existe pas\n";
                    sndmsg(msg2, 8081);
                }else{
                    //write all content in msg
                    fseek(file, 0, SEEK_END);
                    long file_size = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    fread(msg, 1, file_size, file);
                    msg[file_size] = '\0';
                    fclose(file);
                    //send msg
                    printf("Envoi du fichier...\n");
                    sndmsg(msg, 8081);
                    printf("Fichier envoyé\n");
                }
            }
        }else{
            printf("Commande inconnue\n");
        }
    }
    stopserver();
    printf("Fin du server....\n");
}