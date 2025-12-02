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
int utilizador_existe(char *user, Cliente *clientes, int total) {
    int i;
    for (i = 0; i < total; i++)
        if (clientes[i].ativo && strcmp(clientes[i].username, user) == 0)
            return 1;
    return 0;
}

int adiciona_cliente(char *user, Cliente *clientes, char *fifo, int* total) {
    if (total_clientes >= MAX_CLI) return 0;

    strcpy(clientes[*total].username, user);
    strcpy(clientes[*total].fifo_cli, fifo);
    clientes[*total].ativo = true;
    (*total)++;
    return 1;
}
void *thread_clientes(void *arg) {
    PEDIDO p;
    int verifica;
    while (1) {
        if (read(fd_serv, &p, sizeof(PEDIDO)) <= 0)
            continue;

        if (p.tipo == REQ_LOGIN) {
            verifica = 0;

            if (!username_existe(p.username))
                verifica = adiciona_cliente(p.username, p.fifo_cli);

            int fd_cli = open(p.fifo_cli, O_WRONLY);
            if (fd_cli < 0) continue;

            if (verifica)
                write(fd_cli, "LOGIN_OK\n", 9);
            else
                write(fd_cli, "LOGIN_ERRO\n", 11);

            close(fd_cli);
        }

        // restantes pedidos
    }
}
void *thread_admin(void *arg) {
    char cmd[32];

    while (1) {
        scanf("%s", cmd);

        if (strcmp(cmd, "utiliz") == 0) {
            printf("Clientes ativos:\n");
            for (int i = 0; i < MAX_CLI; i++)
                if (clientes[i].ativo)
                    printf("%s\n", clientes[i].username);
            continue;
        }

        if (strcmp(cmd, "terminar") == 0) {
            unlink(FIFO_SERV);
            exit(0);
        }
    }
}


int main(int argc, char *argv[])
{
    int i, nBytes, num_clientes, hora, nveiculos;
    char cmd[TAM_MAX], nome_fifo_cli[25];
    PEDIDO ped;
    RESPOSTA res;
    Cliente cliente;
    Cliente tab_clientes[MAX_CLI];
    pthread_t tid_1;
    char *env = getenv("NVEICULOS");
    if (env == NULL) {
        fprintf(stderr, "Erro: variável de ambiente NVEICULOS não definida\n");
        exit(1);
    }
    nveiculos = atoi(env);
    if (nveiculos <= 0 || nveiculos > MAX_VEICULOS) {
        fprintf(stderr, "Erro: valor inválido de NVEICULOS\n");
        exit(1);
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