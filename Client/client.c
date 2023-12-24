#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "client.h"
#include "server.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <pcre.h>
#include <time.h>
#define AES_KEY_SIZE 256

void removePadding(unsigned char *data, int *length) {
    unsigned char padding = data[*length - 1];
    *length -= padding;
}


int check_input(char *input){
    const char *regex_pattern = "^sectrans -list$|^sectrans -up [a-zA-Z0-9_./]*$|^sectrans -down [a-zA-Z0-9_./]*$";
    pcre *re;
    const char *error;
    int erroffset;
    re = pcre_compile(regex_pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {
        fprintf(stderr, "Erreur de compilation de la regex à la position %d: %s\n", erroffset, error);
        return 1;
    }
    int rc;
    int ovector[30];
    rc = pcre_exec(re, NULL, input, strlen(input), 0, 0, ovector, sizeof(ovector));
    if (rc < 0) {
        if (rc == PCRE_ERROR_NOMATCH) {
            printf("L'entrée ne correspond pas au modèle attendu.\n");
            return 1;
        } else {
            printf("Erreur de correspondance de la regex %d\n", rc);
            return 1;
        }
    } else {
        if(strlen(input) > 1024){
            printf("Votre commande est trop longue, risque de sécurité\n");
            return 1;
        }
    }
    pcre_free(re);
    return 0;
}

unsigned char *generate_key() {
    unsigned char *key = (unsigned char *)malloc(AES_KEY_SIZE / 8);
    FILE *fp;
    fp = fopen("/dev/urandom", "rb");
    if (fp == NULL) {
        perror("Erreur lors de l'ouverture de /dev/urandom");
        exit(EXIT_FAILURE);
    }
    fread(key, 1, AES_KEY_SIZE / 8, fp);
    fclose(fp);
    return key;
}

unsigned char *generate_iv() {
    unsigned char *iv = (unsigned char *)malloc(AES_KEY_SIZE / 8);
    FILE *fp;
    fp = fopen("/dev/urandom", "rb");
    if (fp == NULL) {
        perror("Erreur lors de l'ouverture de /dev/urandom");
        exit(EXIT_FAILURE);
    }
    fread(iv, 1, AES_KEY_SIZE / 8, fp);
    fclose(fp);
    return iv;
}


int main(){
    printf("[+] Bienvenue !  Veuiller rentrer votre commande\n");
    startserver(8081);

    // Clé de chiffrement
    unsigned char *key = generate_key();
    // IV (Initialisation Vector)
    unsigned char *iv = generate_iv();
    // Contexte de chiffrement
    EVP_CIPHER_CTX *ctx;
    // Initialisation du contexte de chiffrement
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    printf("Veuillez rentrer votre identifiant\n");
    char username[1024];
    fgets(username, sizeof(username), stdin);

    printf("Veuillez rentrer votre mot de passe\n");
    char password[1024];
    fgets(password, sizeof(password), stdin);

    char msg[1024] = "connection";
    sndmsg(msg, 8080);
    sndmsg(username, 8080);
    sndmsg(password, 8080);

    char msg2[1024];
    getmsg(msg2);

    if(strncmp(msg2,"Erreur",6) == 0){
        printf("%s", msg2);
        exit(0);
    }else{
        printf("Connexion réussie, vous pouvez rentrer vos commandes\n");
    }

    int number_requests = 0;
    time_t currentTime=time(NULL);
    while(1){

        char input[1024];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if(check_input(input) == 1){
            exit(0);
        }

        number_requests++;

        time_t newTime=time(NULL);
        if(difftime(newTime,currentTime) <= 60 && number_requests > 10){
            printf("Vous avez fait trop de requêtes en peu de temps, veuillez attendre 1 minute, le système se met en pause\n");
            sleep(60);
            number_requests = 0;
            currentTime=time(NULL);
        }else if (difftime(newTime,currentTime) > 60){
            number_requests = 0;
            currentTime=time(NULL);
        }

        int answer = sndmsg(input, 8080);

        if (strncmp(input, "sectrans -list",14) == 0) {
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

                    int myDataSize = strlen(msg);
                    unsigned char outbuf[1024];
                    int outlen;
                    // Chiffrement des données
                    EVP_EncryptUpdate(ctx, outbuf, &outlen, msg, myDataSize);
                    myDataSize += outlen;
                    // Finalisation du chiffrement
                    EVP_EncryptFinal_ex(ctx, outbuf + outlen, &myDataSize);

                    fclose(file);
                    //send msg
                    printf("Envoi du fichier...\n");
                    sndmsg(outbuf, 8080);

                    sndmsg(key, 8080);
                    sndmsg(iv, 8080);

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

                    char key2[1024];
                    getmsg(key2);
                    char iv2[1024];
                    getmsg(iv2);

                    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key2, iv2);
                    int myDataSize = strlen(msg);

                    unsigned char outbuf[1024];
                    int outlen;
                    // Déchiffrement des données
                    EVP_DecryptUpdate(ctx, outbuf, &outlen, msg, myDataSize);
                    myDataSize += outlen;
                    // Finalisation du déchiffrement
                    EVP_DecryptFinal_ex(ctx, outbuf + outlen, &outlen);

                    //print every char of outbuf
                    int size;
                    for (int i = 0; i < myDataSize; ++i) {
                        //printf("%c", outbuf[i]);
                        //if the char is not a printable char
                        if(outbuf[i] < 32 || outbuf[i] > 126){
                            size = i;
                            //printf("\n");
                            break;
                        }
                    }

                    fwrite(outbuf, 1, size, file);
                    fclose(file);
                    printf("Fichier bien reçu\n");
                }
            }
        } else {
            printf("Commande inconnue\n");
        }
    }
}