#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>

// named pipes
#define FIFO_SERV "/tmp/fifo_ctrl"
#define FIFO_CLI_FMT "/tmp/fifo_cli_%s"

// ctes
#define TAM_MAX 256
#define MAX_CLI 30
#define TAM_USER 30
#define MAX_VEI 10
#define NOME_FIFO 256


//tipos de pedido
#define REQ_AGENDAR 1
#define REQ_CANCELAR 2
#define REQ_CONSULTAR 3
#define REQ_TERMINAR 4

typedef struct
{
    char username[TAM_USER];
    //int pid_cli; -acho que nao é preciso
    char fifo_cliente[NOME_FIFO];
    bool ativo;
    // Veiculo *automovel;
} Cliente;


typedef struct
{
    int tipo;                          // REQ_.....
    char username[TAM_USER];
    char fifo_cli[TAM_FIFO];
    int hora;
    int distancia;                 
    int id_servico;
} PEDIDO;

typedef struct
{
    char resposta[TAM_MAX];
    Hora inicio_servico;
    int percorrido;
    bool fim;
} RESPOSTA;


void criaFifo(const char *nome_fifo);
int abreFifo(char *nome_fifo, bool modo);
// int verificaUsername(char *username, Cliente *base, int *total);

#endif