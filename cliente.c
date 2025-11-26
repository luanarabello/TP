#include "cliente.h"

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

int main(int argc, char *argv[])
{
    int fd_cli, fd_serv, nBytes;
    char nome_fifo_cli[25];
    RESPOSTA r;
    PEDIDO p;

    if (access(FIFO_SERV, F_OK))
    {
        printf("ERRO: o controlador nao esta a correr\n");
        exit(3);
    }

    // verificação de user
    if (argc < 2)
    {
        printf("Erro: informe o nome de usuario\n");
        return -1;
    }
    if (strlen(argv[1]) < TAM_USER)
    {
        verificaUsername(argv[1], tab_clientes);
    }
    else
    {
        printf("Username em uso ou muito longo.");
        return -1;
    }

    // abrir fifo do servidor
    fd_serv = open(FIFO_SERV, O_WRONLY);
    if (fd_serv == -1)
    {
        fprintf(stderr, "\nO servidor não está a correr\n");
        unlink(nome_fifo_cli);
        exit(EXIT_FAILURE);
    }

    p.pid_cli = getpid();
    sprintf(nome_fifo_cli, FIFO_CLI, p.pid_cli);
    criaFifo(FIFO_CLI);
    fd_cli = open(FIFO_CLI, O_RDWR);
    if (fd_cli == -1)
    {
        perror("\nErro nao abrir o FIFO do cliente");
        close(fd_serv);
        unlink(nome_fifo_cli);
        exit(EXIT_FAILURE);
    }

    printf("O que gostaria de realizar?\n -agendar\n -cancelar\n -consultar\n -entrar\n -sair\n -terminar\n");

    do
    {

    } while (strcmp(p.cmd, "terminar") != 0);
    exit(0);
}