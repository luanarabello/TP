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

int verificaUsername(char *username, Cliente *base, int *total)
{
    int i;
    for (i = 0; i < MAX_CLI; i++)
    {
        if (base[i].username[0] != '\0' && strcmp(username, base[i].username) == 0)
        {
            // printf("Esse user ja existe. Insira outro:\n");
            return 0;
        }
    }
    if ((*total) < MAX_CLI)
    {
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
        (*total)++;
        return 1;
    }
}

int main(int argc, char *argv[])
{
    int i, nBytes, num_clientes;
    char cmd[TAM_MAX], nome_fifo_cli[25];
    PEDIDO ped;
    RESPOSTA res;
    Cliente cliente;
    Cliente tab_clientes[MAX_CLI];
    pthread_t tid_1;

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
    printf("A espera de clientes...\n");
    fd_s = open(FIFO_SERV, O_RDWR);
    if (fd_s == -1)
    {
        perror("\nErro ao abrir o FIFO do servidor (RDWR/blocking)");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\nFIFO aberto para READ (+WRITE) bloqueante");

    do
    {
        char buffer[sizeof(Cliente) > sizeof(PEDIDO) ? sizeof(Cliente) : sizeof(PEDIDO)];
        nBytes = read(fd_s, buffer, sizeof(buffer));

        if (nBytes < sizeof(ped))
        {
            fprintf(stderr, "\nRecebido pedido incompleto "
                            "[bytes lidos: %d]",
                    nBytes);
            continue; /* não responde a cliente e pula para a próxima iteração */
        }

        if (nBytes == sizeof(Cliente))
        {
            memcpy(&cliente, buffer, sizeof(Cliente));
            fd_c = open(FIFO_CLI, O_WRONLY);
            int valida = verificaUsername(cliente.username, tab_clientes, &num_clientes);
            if (valida == 1)
            {
                char confirmacao[TAM_MAX] = "[CONTROLADOR]: Esse user ja existe. Insira outro:\n";
                write(fd_c, confirmacao, strlen(confirmacao));
            }
            else
            {
                char confirmacao[TAM_MAX] = "[CONTROLADOR]: Cliente cadastrado\n";
                write(fd_c, confirmacao, strlen(confirmacao));
            }
            close(fd_c);
        }

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

    } while (strcmp(cmd, "terminar") != 0);

    close(fd_s);
    unlink(FIFO_SERV);

    exit(0);
}