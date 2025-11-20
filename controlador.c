#include <stdio.h>
#include "uteis.h"

int verificaUsername(char *username, char base[][TAM_USERNAME]){
    int i;
    for(i=0; i<MAX_CLI; i++)
        if(strcmp(username, base[i]) == 0){
            printf("Esse user ja existe. Insira outro:\n");
            return 0;
        }
    return 1;
}


int main(int argc, char *argv[]){
    int fd_servidor, fd_cliente; // file descriptors
    char tab_clientes[MAX_CLI][TAM_USERNAME];
    // strcpy(tab_clientes[0], "Amanda");
    // strcpy(tab_clientes[1], "Jose");
    // strcpy(tab_clientes[2], "Luis");

    char user[TAM_USERNAME];
	printf("Username: ");
	scanf("%s", user);
	int teste = 0;
	do
	{
		teste = verificaUsername(user, tab_clientes);
		if(teste)
			printf("Cliente cadastrado");
		else
		{
			printf("Username: ");
			scanf("%s", user);
		}
	}while (!teste);

    //criarFIFO(FIFO_SERV, 0777); // cria fifo do servidor
    return 0;
}