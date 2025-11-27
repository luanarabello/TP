#include <stdio.h>
#include "uteis.h"

int fd_s, fd_c;

void trataSig(int i)
{
    fprintf(stderr, "\nServidor a terminar "
                    "(interrompido via teclado)\n\n");
    close(fd_s);
    unlink(FIFO_SERV);
    exit(EXIT_SUCCESS); /* para terminar o processo */
}

int main(int argc, char *argv[])
{
    int i, nBytes, num_clientes;
    char cmd[TAM_MAX];
    PEDIDO p;
    RESPOSTA r;
    Cliente tab_clientes[MAX_CLI];

    // só para testar username
    for (i = 0; i < MAX_CLI; i++)
    {
        strcpy(tab_clientes[i].username, "0");
    }

    strcpy(tab_clientes[0].username, "Amanda");
    strcpy(tab_clientes[1].username, "Jose");
    strcpy(tab_clientes[2].username, "Luis");
    //

    if (signal(SIGINT, trataSig) == SIG_ERR)
    {
        perror("\nNao foi possivel configurar o sinal SIGINT\n");
        exit(EXIT_FAILURE);
    }

    criaFifo(FIFO_SERV); // cria fifo do servidor
    printf("A espera de clientes...");
    fd_s = open(FIFO_SERV, O_RDWR);

    do
    {

        scanf("%s", cmd);
        if (strcmp(cmd, "listar") == 0)
            ;
        if (strcmp(cmd, "utiliz") == 0)
        {
            printf("Clientes ativos:\n");
            for (i = 0; i < MAX_CLI && strcmp(tab_clientes[i].username, "0") != 0; ++i)
                printf("%s\n", tab_clientes[i].username);
        }
        if (strcmp(cmd, "frota") == 0)
            ;
        if (strcmp(cmd, "cancelar") == 0)
            ;
        if (strcmp(cmd, "hora") == 0)
            ;

        // nBytes = read(fd_s, &p, sizeof(p));
        // if (nBytes == sizeof(PEDIDO))
        // {
        // }
    } while (strcmp(cmd, "terminar") != 0);

    close(fd_s);
    unlink(FIFO_SERV);

    exit(0);
}