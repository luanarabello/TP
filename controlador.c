#include <stdio.h>
#include "uteis.h"

void verificaUsername(char *username, char base[][TAM_USERNAME]){
    int i;
    for(i=0; i<MAX_CLI; i++){
        if(base[i][0] != '\0' && strcmp(username, base[i]) == 0){
            printf("Esse user ja existe. Insira outro:\n");
            return;
        }
    }
    int pos = -1;
    for(i=0; i<MAX_CLI; i++)
        if(base[i][0] == '\0'){
            pos = i;
            break;
        }
    if(pos != -1){
        strcpy(base[pos], username);
        
    }
    for(i = 0; i < MAX_CLI; i++){
        if(base[i][0] != '\0') // Só imprime os válidos
            printf("[%d]: %s\n", i, base[i]);
    }
}


int main(int argc, char *argv[]){
    int fd_servidor, fd_cliente; // file descriptors
    char tab_clientes[MAX_CLI][TAM_USERNAME] = {0};
    strcpy(tab_clientes[0], "Amanda");
    strcpy(tab_clientes[1], "Jose");
    strcpy(tab_clientes[2], "Luis");
    
    //criarFIFO(FIFO_SERV, 0777); // cria fifo do servidor
    if(argc < 2){
        printf("Erro: informe o nome de usuario\n");
        return -1;
    }
    if(strlen(argv[1])<TAM_USERNAME){
        verificaUsername(argv[1], tab_clientes);
    } else{
        printf("Username em uso ou muito longo.");
    }
    return 0;
}