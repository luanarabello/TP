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
#define MAX_VEICULOS 10
#define NOME_FIFO 256

// tipos de pedido
#define REQ_AGENDAR 1
#define REQ_CANCELAR 2
#define REQ_CONSULTAR 3
#define REQ_TERMINAR 4
#define REQ_LOGIN 5

typedef struct
{
    char username[TAM_USER];
    // int pid_cli; -acho que nao é preciso
    char fifo_cliente[NOME_FIFO];
    bool ativo;
    // Veiculo *automovel;
} Cliente;

typedef struct
{
    int segundos;
} Hora;

typedef struct
{
    Hora inicio_servico;
    int percorrido;
    int estado; // 0: agendado; 1: em curso; 2: concluído/cancelado
    int id;
    char nome_cliente[TAM_USER];
    char local_partida[TAM_MAX];
    int dist_total;
    pid_t pid_cli;
} Servico;

typedef struct
{
    int tipo; // REQ_.....
    char username[TAM_USER];
    char fifo_cli[NOME_FIFO];
    int hora;
    int distancia;
    int id_servico;
    char local_partida[TAM_MAX];
    pid_t pid_cli; // cliente que agenda;
} Pedido;

typedef struct
{
    Cliente *clientes;
} TDATA_ADMIN;

typedef struct
{
    Cliente *clientes;
    int total_clientes;
    Servico *lista_servicos;
    int total_servicos;

} TDATA_CLIENTES;

void criaFifo(const char *nome_fifo);
int abreFifo(char *nome_fifo, bool modo);
// int verificaUsername(char *username, Cliente *base, int *total);

#endif