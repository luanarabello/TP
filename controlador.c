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

int utilizador_existe(char *user, Cliente *clientes, int total)
{
    int i;
    for (i = 0; i < total; i++)
        if (clientes[i].ativo && strcmp(clientes[i].username, user) == 0)
            return 1;
    return 0;
}

int adiciona_cliente(char *user, Cliente *clientes, char *fifo, int *total)
{
    if ((*total) >= MAX_CLI)
        return 0;

    strcpy(clientes[*total].username, user);
    strcpy(clientes[*total].fifo_cliente, fifo);
    clientes[*total].ativo = true;
    (*total)++;
    return 1;
}

void *thread_clientes(void *arg)
{
    TDATA_CLIENTES *ptd = (TDATA_CLIENTES *)arg;
    Pedido p; //?
    int verifica;
    while (1)
    {
        if (read(fd_s, &p, sizeof(Pedido)) <= 0)
            continue;

        if (p.tipo == REQ_LOGIN)
        {
            verifica = 0;

            if (!utilizador_existe(p.username, ptd->clientes, ptd->total_clientes))
                verifica = adiciona_cliente(p.username, ptd->clientes, p.fifo_cli, &ptd->total_clientes);

            int fd_cli = open(p.fifo_cli, O_WRONLY);
            if (fd_cli < 0)
                continue;

            if (verifica)
                write(fd_cli, "LOGIN_OK\n", 9);
            else
                write(fd_cli, "LOGIN_ERRO\n", 11);

            close(fd_cli);
        }

        if (p.tipo == REQ_AGENDAR)
        {
            char resposta[TAM_MAX];

            // verificação para saber se há veiculos disponiveis!!

            Servico *novo = &ptd->lista_servicos[ptd->total_servicos];
            if (novo != NULL)
            {
                novo->id = ptd->total_servicos + 1;
                strcpy(novo->nome_cliente, p.username);
                novo->estado = 0; // agendado...
                novo->percorrido = 0;

                // passados pela linha de args
                novo->inicio_servico.segundos = p.hora;
                novo->dist_total = p.distancia;
                strcpy(novo->local_partida, p.local_partida);

                ptd->total_servicos++;

                // Mensagem de sucesso
                sprintf(resposta, "[CONTROLADOR]: Servico %d agendado com sucesso para a hora %d!\n", novo->id, novo->inicio_servico.segundos);
            }
            else
            {
                sprintf(resposta, "[CONTROLADOR]: Erro ao agendar novo servico\n");
                printf("Erro ao agendar servico");
            }
            int fd_cli;
            fd_cli = abreFifo(p.fifo_cli, true);           // true é para escrita
            write(fd_cli, resposta, strlen(resposta) + 1); // para incluir o '\0'
            close(fd_cli);
        }

        if (p.tipo == REQ_CONSULTAR)
        {
            int fd_cli, i;

            size_t tam_resposta = TAM_MAX;
            char *resposta = malloc(tam_resposta);
            char temp[TAM_MAX];

            if (resposta == NULL)
            {
                perror("Erro ao alocar memória para resposta");
                continue;
            }
            resposta[0] = '\0';

            fd_cli = abreFifo(p.fifo_cli, true);

            sprintf(resposta, "\n--- Serviços Agendados para %s ---\n", p.username);

            for (i = 0; i < ptd->total_servicos; i++)
            {
                Servico *s = &ptd->lista_servicos[i];

                if (strcmp(s->nome_cliente, p.username) == 0)
                { // achar os serviços solicitados pelo cliente que fez o pedido
                    char *info_estado = (s->estado == 0) ? "Agendado" : (s->estado == 1) ? "Em curso"
                                                                                         : "Concluido";
                    sprintf(temp, "ID do Servico: %d | Hora: %d | Local de Partida: %s | Distância Total: %d km | Estado: %s\n", s->id, s->inicio_servico.segundos, s->local_partida, s->dist_total, info_estado);

                    size_t novo_tam_resposta = strlen(resposta) + strlen(temp) + 1; // +1 pro '\0'

                    // verificação de overflow do buffer
                    if (novo_tam_resposta > tam_resposta)
                    {
                        char *nova_resposta = realloc(resposta, novo_tam_resposta);

                        if (nova_resposta == NULL)
                        {
                            perror("Erro ao realocar memória para resposta - resposta truncada.");
                            close(fd_cli);
                            free(resposta);
                            // Deve-se fechar o FIFO e dar free em resposta antes de sair
                            // ...
                            break;
                        }

                        resposta = nova_resposta;
                        tam_resposta = novo_tam_resposta;
                    }
                    strcat(resposta, temp);
                }
            }
            write(fd_cli, resposta, strlen(resposta) + 1);
            close(fd_cli);
            free(resposta);
        }

        if (p.tipo == REQ_CANCELAR)
        {
            Servico *aux = ptd->lista_servicos;
            int i, n, fd_cli, serv_cancelados = 0;
            char resposta[TAM_MAX];
            bool encontrado = false;
            fd_cli = abreFifo(p.fifo_cli, true);

            for (i = 0; i < ptd->total_servicos; i++)
            {
                if (strcmp(aux[i].nome_cliente, p.username) == 0)
                {
                    encontrado = true;
                    if (p.id_servico == 0)
                    { // cancelar todos
                        if (aux[i].estado == 0)
                        {
                            aux[i].estado = 2;
                            serv_cancelados++;
                        }
                    }
                    else if (aux[i].id == p.id_servico)
                    {
                        if (aux[i].estado == 0)
                        {
                            aux[i].estado = 2; // cancelado/concluido
                            serv_cancelados++;
                            sprintf(resposta, "[CONTROLADOR]: Serviço de id %d cancelado com sucesso.\n", aux[i].id);
                            break;
                        }
                        else if (aux[i].estado == 1) // servico em curso nao podem ser cancelados
                        {
                            sprintf(resposta, "[CONTROLADOR]: Erro: Serviço %d esta em curso e não pode ser cancelado.\n", p.id_servico);
                        }
                        else if (aux[i].estado == 2) // servico ja foi cancelado
                        {
                            sprintf(resposta, "[CONTROLADOR]: Erro: Serviço %d ja se encontra cancelado ou concluido.\n", p.id_servico);
                            break;
                        }
                    }
                }
            }

            if (strlen(resposta) == 0)
            {
                if (p.id_servico == 0)
                {
                    if (encontrado)
                        sprintf(resposta, "[CONTROLADOR]: Todos os serviços foram cancelados com sucesso.\n");
                    else
                        sprintf(resposta, "[CONTROLADOR]: Nao ha serviços agendados.");
                }
                else
                {
                    sprintf(resposta, "[CONTROLADOR]: Erro: servico %d nao encontrado ou nao pertence a utilizador %s", p.id_servico, p.username);
                }
            }

            n = write(fd_cli, resposta, strlen(resposta) + 1);
            if (n != strlen(resposta) + 1)
            {
                fprintf(stderr, "Erro: mensagem truncada.");
            }
            close(fd_cli);
        }

        if (p.tipo == REQ_TERMINAR)
        {
            ;
        }
    }

    // restantes pedidos
}

void *thread_admin(void *arg)
{
    TDATA_ADMIN *ptd = (TDATA_ADMIN *)arg;
    char cmd[32];

    while (1)
    {
        scanf("%s", cmd);

        if (strcmp(cmd, "utiliz") == 0)
        {
            printf("Clientes ativos:\n");
            for (int i = 0; i < MAX_CLI; i++)
                if (ptd->clientes[i].ativo)
                    printf("%s\n", ptd->clientes[i].username);
            continue;
        }

        if (strcmp(cmd, "terminar") == 0)
        {
            unlink(FIFO_SERV);
            exit(0);
        }
    }
}

int main(int argc, char *argv[])
{
    int i, nBytes, num_clientes, hora, nveiculos;
    char cmd[TAM_MAX], nome_fifo_cli[25];
    Pedido ped;
    Cliente cliente;
    Cliente tab_clientes[MAX_CLI];

    pthread_t tid_1, tid_2;
    TDATA_ADMIN t1_data;    // admin
    TDATA_CLIENTES t2_data; // user
    t1_data.clientes = tab_clientes;

    t2_data.clientes = tab_clientes;
    t2_data.total_clientes = sizeof(tab_clientes) / sizeof(Cliente);

    pthread_create(&tid_1, NULL, thread_admin, (void *)&t1_data);
    pthread_create(&tid_2, NULL, thread_clientes, (void *)&t2_data);

    if (signal(SIGINT, trataSig) == SIG_ERR)
    {
        perror("\nNao foi possivel configurar o sinal SIGINT\n");
        exit(EXIT_FAILURE);
    }

    char *env = getenv("NVEICULOS"); // env é envio
    if (env == NULL)
    {
        fprintf(stderr, "Erro: variável de ambiente NVEICULOS não definida\n");
        exit(1);
    }
    nveiculos = atoi(env);
    if (nveiculos <= 0 || nveiculos > MAX_VEICULOS)
    {
        fprintf(stderr, "Erro: valor inválido de NVEICULOS\n");
        exit(1);
    }

    strcpy(tab_clientes[0].username, "Amanda");
    strcpy(tab_clientes[1].username, "Jose");
    strcpy(tab_clientes[2].username, "Luis");
    //

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
        char buffer[sizeof(Cliente) > sizeof(Pedido) ? sizeof(Cliente) : sizeof(Pedido)];
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
            fd_c = open(FIFO_CLI_FMT, O_WRONLY);
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