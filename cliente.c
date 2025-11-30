#include "uteis.h"
// #include "cliente.h"

int main(int argc, char *argv[])
{
    int fd_cli, fd_serv, nBytes;
    char nome_fifo_cli[25];
    RESPOSTA r;
    PEDIDO p;
    Cliente cliente;

    if (access(FIFO_SERV, F_OK))
    {
        printf("ERRO: o controlador nao esta a correr\n");
        exit(3);
    }

    fd_serv = open(FIFO_SERV, O_WRONLY);
    if (fd_serv == -1)
    {
        fprintf(stderr, "\nO servidor não está a correr\n");
        unlink(nome_fifo_cli);
        exit(EXIT_FAILURE);
    }

    if (argc != 2)
    {
        printf("Uso: ./cliente <username>");
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
        ;
    } while (strcmp(p.cmd, "terminar") != 0);
    close(fd_cli);
    unlink(FIFO_CLI);
    exit(0);
}