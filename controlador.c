#include <stdio.h>
#include "uteis.h"

int main(int argc, char *argv[])
{
    int i, fd_s, fd_c, nBytes; // file descriptors
    char cmd[TAM_MAX];
    PEDIDO p;
    RESPOSTA r;

    for (i = 0; i < MAX_CLI; i++)
    {
        strcpy(tab_clientes[i].username, "0");
    }

    strcpy(tab_clientes[0].username, "Amanda");
    strcpy(tab_clientes[1].username, "Jose");
    strcpy(tab_clientes[2].username, "Luis");

    criaFifo(FIFO_SERV); // cria fifo do servidor
    // fd_s = open(FIFO_SERV, O_RDWR);

    do
    {
        if (strcmp(cmd, "listar"))
            ;
        if (strcmp(cmd, "utiliz"))
        {
            for (i = 0; i < MAX_CLI && strcmp(tab_clientes[i].username, "0") != 0; ++i)
                printf("Clientes ativos:\n %s\n", tab_clientes[i].username);
        }
        if (strcmp(cmd, "frota"))
            ;
        if (strcmp(cmd, "cancelar"))
            ;
        if (strcmp(cmd, "hora"))
            ;

        nBytes = read(fd_s, &p, sizeof(p));
        if (nBytes == sizeof(PEDIDO))
        {
        }
    } while (strcmp(cmd, "terminar") != 0);
    return 0;
}