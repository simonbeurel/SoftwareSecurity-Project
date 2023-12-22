#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "client.h"
#include "server.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#define AES_KEY_SIZE 128

void removePadding(unsigned char *data, int *length) {
    unsigned char padding = data[*length - 1];
    *length -= padding;
}


int main(){
    printf("[+] Bienvenue !  Veuiller rentrer votre commande\n");
    startserver(8081);

    // Clé de chiffrement
    unsigned char *key = (unsigned char *)"01234567890123456789012345678901";
    // IV (Initialisation Vector)
    unsigned char *iv = (unsigned char *)"1234567890123456";
    // Contexte de chiffrement
    EVP_CIPHER_CTX *ctx;
    // Initialisation du contexte de chiffrement
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);


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

                    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
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
                        //if the char is not a printable char
                        if(outbuf[i] < 32 || outbuf[i] > 126){
                            size = i;
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