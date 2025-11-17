#include "uteis.h"

FIFO criaFifo(char *nome_fifo, int permissao){
    int res;
    if(permissao == 0666)
        res = mkfifo(nome_fifo, 0666) //0666 é permissão para leitura e escrita a tds utilizadores
    if(permissao == 0777)
        res = mkfifo(nome_fifo, 0777) //0666 é permissão para leitura, escrita e execução ao proprietário
    if(res == -1){
        perror("\nErro ao criar FIFO");
        exit(EXIT_FAILURE);
    }
    
}