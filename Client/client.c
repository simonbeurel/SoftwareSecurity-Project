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
        } else {
            printf("Commande inconnue\n");
        }
    }
}