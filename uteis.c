#include "uteis.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void criaFifo(const char *nome_fifo)
{
    int res;
    res = mkfifo(nome_fifo, 0640);
    if (res < 0)
    {
        perror("\nErro ao criar FIFO");
        exit(EXIT_FAILURE);
    }
}

int abreFifo(char *nome_fifo, bool modo)
{
    int fd;
    if (modo == true)
        fd = open(nome_fifo, O_WRONLY);
    else
        fd = open(nome_fifo, O_RDONLY);
    if (fd < 0)
    {
        perror("\nErro ao criar FIFO");
        unlink(nome_fifo);
        exit(EXIT_FAILURE);
    }
    return fd;
}

void verificaUsername(char *username, Cliente *base)
{
    int i;
    for (i = 0; i < MAX_CLI; i++)
    {
        if (base[i].username[0] != '\0' && strcmp(username, base[i].username) == 0)
        {
            printf("Esse user ja existe. Insira outro:\n");
            return;
        }
    }
    printf("\nUsuario cadastrado!\n");
    for (i = 0; i < MAX_CLI; i++)
        if (strcmp(base[i].username, "0") == 0)
        {
            strcpy(base[i].username, username);
            break;
        }

    printf("\n--- Tabela Atualizada ---\n");
    for (i = 0; i < MAX_CLI; i++)
    {
        if (strcmp(base[i].username, "0") != 0)
            printf("[%d]: %s\n", i, base[i].username);
    }
}