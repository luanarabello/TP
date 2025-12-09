#include "uteis.h"

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
        perror("\nErro ao abrir FIFO");
        unlink(nome_fifo);
        exit(EXIT_FAILURE);
    }
    return fd;
}
