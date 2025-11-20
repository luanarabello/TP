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

//named pipes
#define FIFO_SERV "/tmp/dict_fifo" 
#define FIFO_CLI "/tmp/resp_%d_fifo"

//ctes
#define TAM_MAX 256
#define MAX_CLI 20
#define TAM_USERNAME 30


typedef struct {
    pid_t pid_cliente;
    char comando[TAM_MAX];
} PEDIDO;


typedef struct{
    char resposta[TAM_MAX];
}RESPOSTA;

void criaFifo(char *nome_fifo, int permissao);
int abreFifo(char *nome_fifo, bool modo);

