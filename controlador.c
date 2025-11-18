#include <stdio.h>
#include "uteis.h"

bool verificaUsername(char *username, char *base){
    int i;
    for(i=0; i<MAX_CLI; i++)
        if(strcmp(username, base[i][j]) == 0){
            printf("Esse user ja existe. Insira outro:\n");
            return false;
        }
    return true;
}


int main(int argc, char *argv[]){
    int fd_servidor, fd_cliente; // file descriptors
    char tab_clientes[MAX_CLI][TAM_USERNAME];
    strcpy(tab_clientes[0], "Amanda");
    strcpy(tab_clientes[1], "Jose");
    strcpy(tab_clientes[2], "Luis");

    char *user;
    printf("Username: ");
    scanf("%s", &user)
    int teste = verificaUsername(user, tab_clientes);
    if(teste)
        printf("true");
    else
        printf("false");

    //criarFIFO(FIFO_SERV, 0777); // cria fifo do servidor
    return 0;
}