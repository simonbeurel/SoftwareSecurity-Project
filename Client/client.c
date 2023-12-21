#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "client.h"
#include "server.h"

int main(){
    printf("[+] Bienvenue !  Veuiller rentrer votre commande\n");
    startserver(8081);
    while(1){
        char input[1024];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        int answer = sndmsg(input, 8080);

        if (strcmp(input, "sectrans -list") == 0) {
            char msg[1024];
            getmsg(msg);
            printf("%s\n", msg);
        } else if (strncmp(input, "sectrans -up", 12) == 0) {
            char *copy = strdup(input);
            char *saveptr;
            char *tokenized = strtok_r(copy, " ", &saveptr);
            for (int i = 0; i < 2 && tokenized != NULL; ++i) {
                tokenized = strtok_r(NULL, " ", &saveptr);
            }
            if (tokenized != NULL) {
                char msg[1024];
                //open the file "tokenized"
                FILE *file = fopen(tokenized, "r");
                if (file == NULL) {
                    printf("Erreur lors de l'ouverture du fichier, votre fichier n'existe pas dans le bon chemin\n");
                    char msg2[1024] = "ERROR";
                    sndmsg(msg2, 8080);
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
                    sndmsg(msg, 8080);
                    printf("Fichier envoyé\n");
                    char msg2[1024];
                    getmsg(msg2);
                    printf("%s\n", msg2);
                }
            }
        }else if (strncmp(input,"sectrans -down", 14) == 0){
            printf("Reception dun fichier\n");
            char *copy = strdup(input);
            char *saveptr;
            char *tokenized = strtok_r(copy, " ", &saveptr);
            for (int i = 0; i < 2 && tokenized != NULL; ++i) {
                tokenized = strtok_r(NULL, " ", &saveptr);
            }
            if (tokenized != NULL) {
                char msg[1024];
                getmsg(msg);
                if(strncmp(msg,"Erreur",6) == 0){
                    printf("%s", msg);
                }else{
                    FILE *file = fopen(tokenized, "w");
                    //write all msg in the new file
                    fwrite(msg, 1, strlen(msg), file);
                    fclose(file);
                    printf("Fichier bien reçu\n");
                }
            }
        } else {
            printf("Commande inconnue\n");
        }
    }
}