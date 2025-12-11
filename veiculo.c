#include "uteis.h"

int main(int argc, char *argv[])
{
    // Argumentos esperados: ./veiculo <id_servico> <fifo_cliente> <distancia>
    if (argc < 4) {
        fprintf(stderr, "[VEICULO] Erro: Argumentos insuficientes.\n");
        return 1;
    }

    int id_servico = atoi(argv[1]);
    char *fifo_cli = argv[2];
    int distancia_total = atoi(argv[3]);
    int percorrido = 0;
    int percentagem = 0;
    int fd_cli;

    // avisar o cliente que o veículo chegou
    // (Abre o FIFO do cliente para escrita)
    fd_cli = open(fifo_cli, O_WRONLY);
    if (fd_cli == -1) {
        fprintf(stderr, "[VEICULO %d] Erro ao contactar cliente no fifo: %s\n", id_servico, fifo_cli);
        // Não faz exit porque o controlador precisa de saber que o veículo correu
    } else {
        char msg[TAM_MAX];
        sprintf(msg, "VEICULO %d CHEGOU! Entre para iniciar.", id_servico);
        write(fd_cli, msg, strlen(msg)+1);
        close(fd_cli);
    }

    
    fprintf(stdout, "INICIO %d %d\n", id_servico, distancia_total); // Avisa controlador
    fflush(stdout); // Obriga a escrever logo

    while (percorrido < distancia_total) {
        sleep(1); // Espera 1 segundo (simula tempo a andar)
        percorrido++;

        int nova_perc = (percorrido * 100) / distancia_total;

        // a cada 10% avisa o controlador
        if (nova_perc / 10 > percentagem / 10) {
            fprintf(stdout, "STATUS %d %d\n", id_servico, nova_perc);
            fflush(stdout);
        }
        percentagem = nova_perc;
    }

    // fim da viagem
    fprintf(stdout, "CONCLUIDO %d\n", id_servico);
    fflush(stdout);

    // Avisar cliente final cagativo mas fica bonitinho
    fd_cli = open(fifo_cli, O_WRONLY);
    if (fd_cli != -1) {
        write(fd_cli, "VIAGEM TERMINADA", 17);
        close(fd_cli);
    }

    return 0;
}