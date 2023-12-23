#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include "client.h"
#include "server.h"

struct user{
    char *name;
    char *password;
    struct user *next;
};

struct public_key{
    char *name;
    char *key;
    char *iv;
    struct public_key *next;
};

void addkey(struct public_key *key, char *name, char *key2, char *iv2){
    struct public_key *index = key;
    while(index->next != NULL){
        //if the key already exist, we update it
        if(strncmp(index->name,name,strlen(index->name)) == 0){
            index->key = strdup(key2);
            index->iv = strdup(iv2);
            return;
        }
        index = index->next;
    }
    struct public_key *new_key = malloc(sizeof(struct public_key));
    new_key->name = strdup(name);
    new_key->key = strdup(key2);
    new_key->iv = strdup(iv2);
    new_key->next = NULL;
    index->next = new_key;
}

struct public_key *getkey(struct public_key *key, char *name){
    struct public_key *index = key;
    while(index != NULL){
        if(strncmp(index->name,name,strlen(index->name)) == 0){
            return index;
        }
        index = index->next;
    }
    return NULL;
}



int main(){
	startserver(8080);
    printf("Serveur activé...\n");

    //create the first key
    struct public_key *key = malloc(sizeof(struct public_key));
    key->name = strdup("NULL");
    key->key = strdup("NULL");
    key->iv = strdup("NULL");
    key->next = NULL;

    //create the first user admin admin
    struct user *admin = malloc(sizeof(struct user));
    admin->name = strdup("admin");
    admin->password = strdup("admin");

    struct user *new_user = malloc(sizeof(struct user));
    new_user->name = strdup("test");
    new_user->password = strdup("test");
    admin->next = new_user;

    while(1){

        char rcv_msg[1024];
        getmsg(rcv_msg);

        printf("Message reçu :%s\n",rcv_msg);

        if(strncmp(rcv_msg,"exit", 4) == 0){
            break;
        }
        else if (strncmp(rcv_msg, "connection",10) == 0) {
            char user_test[1024];
            getmsg(user_test);
            char password_test[1024];
            getmsg(password_test);
            struct user *index = admin;
            int found = 0;
            while(index != NULL){
                if(strncmp(index->name,user_test,strlen(index->name)) == 0 && strncmp(index->password,password_test,strlen(index->password)) == 0){
                    printf("Connexion réussie\n");
                    char msg[1024] = "Connexion réussie";
                    sndmsg(msg,8081);
                    found = 1;
                    break;
                }
                index = index->next;
            }
            if(found==0){
                printf("Erreur : Identifiant ou mot de passe incorrect\n");
                char msg[1024] = "Erreur : Identifiant ou mot de passe incorrect";
                sndmsg(msg,8081);
            }
        }
        else if (strncmp(rcv_msg,"sectrans -list", 14) == 0) {
            printf("Liste des fichiers demandée...\n");
            char msg[1024] = "";
            strcat(msg, "Liste des fichiers disponibles :\n");
            DIR *dir;
            struct dirent *entry;
            dir = opendir("./database");
            if (dir == NULL) {
                perror("Erreur lors de l'ouverture du répertoire\n");
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

                    //get the key and iv
                    char key2[1024];
                    getmsg(key2);
                    char iv2[1024];
                    getmsg(iv2);
                    addkey(key,tokenized,key2,iv2);

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
                    //get the key and iv
                    struct public_key *key2 = getkey(key,tokenized);
                    sndmsg(key2->key, 8081);
                    sndmsg(key2->iv, 8081);
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