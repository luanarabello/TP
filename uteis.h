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
#define FIFO_SERV "/tmp/dict_fifo"
#define FIFO_CLI "/tmp/resp_%d_fifo"

// ctes
#define TAM_MAX 256
#define MAX_CLI 30
#define TAM_USER 30
#define MAX_VEICULOS 10
#define NOME_FIFO 256

typedef struct
{
    char username[TAM_USER];
    int pid_cli;
    char fifo_cliente[NOME_FIFO];
    // Veiculo *automovel;
} Cliente;

typedef struct
{
    int horas, minutos, segundos;
} Hora;

typedef struct
{
    char cmd[TAM_MAX];
    char *partida, *destino;
    Hora hora;
    int distancia;
    int id, pid_cli;
    char username[TAM_USER];

} PEDIDO;

typedef struct
{
    char resposta[TAM_MAX];
    Hora inicio_servico;
    int percorrido;
    bool fim;
} RESPOSTA;

typedef struct
{
    int continua;
} TDATA;

void criaFifo(const char *nome_fifo);
int abreFifo(char *nome_fifo, bool modo);
// int verificaUsername(char *username, Cliente *base, int *total);