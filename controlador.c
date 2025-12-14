#include "uteis.h"
#include <pthread.h>

int fd_s;
pthread_mutex_t trinco = PTHREAD_MUTEX_INITIALIZER;
int total_kms=0;    
int horas=0;
int nveiculos = 0;

Cliente tab_clientes[MAX_CLI];
int total_clientes = 0;

Servico tab_servicos[TAM_MAX];
int total_servicos = 0;

void trataSig(int i)
{
    (void)i;
    fprintf(stderr, "\n[CONTROLADOR] A encerrar via sinal...\n");
    printf("Admin > ");
    close(fd_s);
    unlink(FIFO_SERV);
    exit(EXIT_SUCCESS);
}
void lancar_veiculo(Servico *s, char *fifo_cli)
{
    int p[2];

    if (pipe(p) == -1)
    {
        perror("\n[ERRO] Pipe falhou");
        printf("Admin > ");
        return;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("\n[ERRO] Fork falhou");
        printf("Admin > ");
        return;
    }

    if (pid == 0)
    { //filho
        close(p[0]);
        dup2(p[1], STDOUT_FILENO);
        close(p[1]);

        char str_id[10], str_dist[10];
        sprintf(str_id, "%d", s->id);
        sprintf(str_dist, "%d", s->dist_total);

        execl("./veiculo", "./veiculo", str_id, fifo_cli, str_dist, NULL);
        exit(1);
    }
    else
    { // pai
        close(p[1]);
        s->pipe_fd = p[0];
        s->pid_veiculo = pid;
        s->estado = 1; // Em Curso

        int flags = fcntl(s->pipe_fd, F_GETFL, 0);
        fcntl(s->pipe_fd, F_SETFL, flags | O_NONBLOCK);

        printf("\n[SISTEMA] Veiculo lançado (PID: %d)\n", pid);
        printf("Admin > ");
    }
}
void *thread_relogio()
{
    char fifo_cli[TAM_MAX];

    while (1)
    {
        sleep(1); 
        
        pthread_mutex_lock(&trinco);
        horas++;

        int ativos = 0;
        for (int k = 0; k < total_servicos; k++) 
            if (tab_servicos[k].estado == 1) ativos++;
        

        // verificar se há serviços agendados para esta hora (ou atrasados)
        for (int k = 0; k < total_servicos; k++)
        {
            // Se está Agendado (0) E já chegou a hora
            if (tab_servicos[k].estado == 0 && 
                tab_servicos[k].inicio_servico.segundos <= horas)
            {
                if (ativos < nveiculos) // Temos carros livres
                {
                                      
                    // Lança o veículo
                    lancar_veiculo(&tab_servicos[k], tab_servicos[k].fifo_cliente);
                }

            }
        }
        pthread_mutex_unlock(&trinco);
    }
    return NULL;
}
int utilizador_existe(char *user, Cliente *clientes, int total)
{
    for (int i = 0; i < total; i++)
        if (clientes[i].ativo && strcmp(clientes[i].username, user) == 0)
            return 1;
    return 0;
}

int adiciona_cliente(char *user, Cliente *clientes, char *fifo, int *total)
{
    if ((*total) >= MAX_CLI)
    {
        printf("\n[ERRO] Lista cheia\n");
        printf("Admin > ");
        return 0;
    }
    strcpy(clientes[*total].username, user);
    strcpy(clientes[*total].fifo_cliente, fifo);
    clientes[*total].ativo = true;
    (*total)++;
    return 1;
}


// monitoriza os veículos constantemente 
void *thread_telemetria(void *arg)
{
    TDATA_CLIENTES *ptd = (TDATA_CLIENTES *)arg;
    char buffer_veiculo[100];
    int id_lido, valor_lido;

    while (1)
    {
        for (int k = 0; k < ptd->total_servicos; k++)
        {
            // Só monitoriza veículos em movimento (Estado 1)
            if (ptd->lista_servicos[k].estado == 1)
            {
                int n = read(ptd->lista_servicos[k].pipe_fd, buffer_veiculo, sizeof(buffer_veiculo) - 1);
                
                if (n > 0)
                {
                    buffer_veiculo[n] = '\0';

                    if (sscanf(buffer_veiculo, "STATUS %d %d", &id_lido, &valor_lido) == 2)
                    {
                        pthread_mutex_lock(&trinco); // Protege a escrita nas globais

                        ptd->lista_servicos[k].percorrido = valor_lido; 
                        
                        //soma 10% da distância a cada aviso
                        total_kms += (int)(ptd->lista_servicos[k].dist_total * 0.1);

                        pthread_mutex_unlock(&trinco);
                    }

                    printf("\r[VEICULO %d]: %sAdmin > ", ptd->lista_servicos[k].id, buffer_veiculo);
                    fflush(stdout);
                }
                else if (n == 0) // Veículo acabou
                {
                    pthread_mutex_lock(&trinco);
                    
                    ptd->lista_servicos[k].estado = 2; // Concluído
                    
                    pthread_mutex_unlock(&trinco);
                    
                    close(ptd->lista_servicos[k].pipe_fd);
                    ptd->lista_servicos[k].pipe_fd = -1;
                }
            }
        }
        usleep(100000); 
    }
    return NULL;
}

void *thread_clientes(void *arg)
{
    TDATA_CLIENTES *ptd = (TDATA_CLIENTES *)arg;
    Pedido p;
    int fd_cli;

    printf("\n[THREAD] Thread de clientes a correr...\n");
    printf("Admin > ");

    while (1)
    {
        int n = read(fd_s, &p, sizeof(Pedido));
        if (n <= 0)
        {
            usleep(1000);
            continue;
        }

        if (p.tipo == REQ_LOGIN)
        {
            pthread_mutex_lock(&trinco);
            int existe = utilizador_existe(p.username, ptd->clientes, ptd->total_clientes);
            int verifica = 0;
            if (!existe)
                verifica = adiciona_cliente(p.username, ptd->clientes, p.fifo_cli, &ptd->total_clientes);
            pthread_mutex_unlock(&trinco);

            fd_cli = open(p.fifo_cli, O_WRONLY);
            if (fd_cli != -1)
            {
                if (verifica)
                {
                    write(fd_cli, "LOGIN_OK", 9);
                    printf("\n[LOGIN] Novo: %s\n", p.username);
                    printf("Admin > ");
                }
                else
                {
                    write(fd_cli, "LOGIN_ERRO", 11);
                }
                close(fd_cli);
            }
        }
        else if (p.tipo == REQ_AGENDAR)
        {
            char resposta[TAM_MAX];
            pthread_mutex_lock(&trinco);
            if (ptd->total_servicos < TAM_MAX)
            {
                Servico *novo = &ptd->lista_servicos[ptd->total_servicos];
                novo->id = ptd->total_servicos + 1;
                strcpy(novo->nome_cliente, p.username);
                strcpy(novo->fifo_cliente, p.fifo_cli);
                novo->estado = 0;
                novo->percorrido = 0;
                novo->inicio_servico.segundos = p.hora;
                novo->dist_total = p.distancia;
                strcpy(novo->local_partida, p.local_partida);
                sprintf(resposta, "Agendado (ID %d) para T=%d.", novo->id, p.hora);

                total_servicos++;
                ptd->total_servicos = total_servicos;
            }
            else
            {
                sprintf(resposta, "Erro: Frota cheia.");
            }
            pthread_mutex_unlock(&trinco);

            fd_cli = open(p.fifo_cli, O_WRONLY);
            if (fd_cli != -1)
            {
                write(fd_cli, resposta, strlen(resposta) + 1);
                close(fd_cli);
            }
        }
        else if (p.tipo == REQ_CONSULTAR)
        {
            char lista_completa[TAM_MAX * 5] = "";
            char linha[200];
            int encontrou = 0;

            sprintf(lista_completa, "\nServicos\n");
            for (int k = 0; k < ptd->total_servicos; k++)
            {
                if (strcmp(ptd->lista_servicos[k].nome_cliente, p.username) == 0 && ptd->lista_servicos[k].estado == 0)
                {
                    sprintf(linha, "ID: %d | Origem: %s | Estado: Agendado\n",
                            ptd->lista_servicos[k].id, ptd->lista_servicos[k].local_partida);
                    strcat(lista_completa, linha);
                    encontrou = 1;
                }
            }
            if (!encontrou)
                strcat(lista_completa, "Nenhum servico.\n");

            fd_cli = open(p.fifo_cli, O_WRONLY);
            if (fd_cli != -1)
            {
                write(fd_cli, lista_completa, strlen(lista_completa) + 1);
                close(fd_cli);
            }
        }
        else if (p.tipo == REQ_CANCELAR)
        {
            char res[TAM_MAX];
            int cancelados = 0;

            pthread_mutex_lock(&trinco);

            for (int k = 0; k < ptd->total_servicos; k++)
            {
                Servico *s = &ptd->lista_servicos[k];
                if (strcmp(s->nome_cliente, p.username) == 0)
                {
                    if (p.id_servico == 0 || s->id == p.id_servico)
                    {
                        if (s->estado == 1)
                        {
                            kill(s->pid_veiculo, SIGUSR1);
                            s->estado = 2;
                            cancelados++;
                        }
                    }
                }
            }

            pthread_mutex_unlock(&trinco);

            sprintf(res, "Cancelados: %d", cancelados);
            fd_cli = open(p.fifo_cli, O_WRONLY);
            if (fd_cli != -1)
            {
                write(fd_cli, res, strlen(res) + 1);
                close(fd_cli);
            }
        }
        else if (p.tipo == REQ_TERMINAR)
        {
            char resposta[TAM_MAX];
            int tem_viagem_em_curso = 0;

            pthread_mutex_lock(&trinco); // Bloqueia

            //verificar se cliente existe e se tem viagens a andar 1
            for (int k = 0; k < ptd->total_servicos; k++)
            {
                if (strcmp(ptd->lista_servicos[k].nome_cliente, p.username) == 0 && 
                    ptd->lista_servicos[k].estado == 1)
                {
                    tem_viagem_em_curso = 1;
                    break; 
                }
            }

            if (tem_viagem_em_curso)
            {
                
                sprintf(resposta, "ERRO: Tem uma viagem em curso. Aguarde que termine.");
            }
            else
            {
            
                sprintf(resposta, "OK_SAIR");

                // Encontrar o cliente e marcar como inativo
                for (int i = 0; i < ptd->total_clientes; i++)
                {
                    if (strcmp(ptd->clientes[i].username, p.username) == 0)
                    {
                        ptd->clientes[i].ativo = false;
                        printf("\n[LOGOUT] Utilizador saiu: %s\n", p.username);
                        printf("Admin > ");
                        break;
                    }
                }

                // Cancelar  os agendados
                
                for (int k = 0; k < ptd->total_servicos; k++)
                {
                    if (strcmp(ptd->lista_servicos[k].nome_cliente, p.username) == 0 && 
                        ptd->lista_servicos[k].estado == 0) 
                    {
                        ptd->lista_servicos[k].estado = 2; // Cancelado
                        printf("\n[AUTO-CANCEL] Serviço %d (Agendado) cancelado.\n", ptd->lista_servicos[k].id);
                        printf("Admin > ");
                    }
                }
            }
            
            pthread_mutex_unlock(&trinco); 

        
            fd_cli = open(p.fifo_cli, O_WRONLY);
            if (fd_cli != -1)
            {
                write(fd_cli, resposta, strlen(resposta) + 1);
                close(fd_cli);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int i;
    char cmd[TAM_MAX];
    char mensagem[TAM_MAX];


    pthread_t tid_cli, tid_tel, tid_relogio;
    TDATA_CLIENTES t2_data;
    setbuf(stdout, NULL); //para nao bloquear no scanf e fazer logo as validações
    memset(tab_clientes, 0, sizeof(tab_clientes));
    memset(tab_servicos, 0, sizeof(tab_servicos));

    if (signal(SIGINT, trataSig) == SIG_ERR)
    {
        perror("Erro sinal");
        exit(1);
    }

    char *env = getenv("NVEICULOS");
    if (env == NULL) {
        fprintf(stderr, "[ERRO] A variavel NVEICULOS nao esta definida!\n");
        return 1;
    }
    nveiculos = atoi(env);
    if (access(FIFO_SERV, F_OK) == 0) 
    {
        fprintf(stderr, "[ERRO] O controlador ja esta a correr (ou o FIFO '%s' nao foi apagado)!\n", FIFO_SERV);
        fprintf(stderr, "Apague o FIFO manualmente se nao houver nenhum controlador ativo.\n");
        exit(1);
    }



    t2_data.clientes = tab_clientes;
    t2_data.total_clientes = 0;
    t2_data.lista_servicos = tab_servicos;
    t2_data.total_servicos = 0;

    criaFifo(FIFO_SERV);

    fd_s = open(FIFO_SERV, O_RDWR);
    if (fd_s == -1)
    {
        perror("Erro FIFO");
        exit(1);
    }
    // Lançar Threads
    pthread_create(&tid_cli, NULL, thread_clientes, (void *)&t2_data);

    pthread_create(&tid_tel, NULL, thread_telemetria, (void *)&t2_data);
    pthread_create(&tid_relogio, NULL, thread_relogio, NULL);

    printf("\nControlador iniciado (NVeiculos: %d). Aguardando...\n", nveiculos);


    while (1)
    {
        printf("Admin > ");

        scanf("%s", cmd); // Scanf bloqueia aqui, mas as threads continuam a correr!
        if (strcmp(cmd, "hora") == 0)
        {
            printf("Hora do sistema: %d\n", horas);
        }
        else if (strcmp(cmd, "km") == 0)
        {
            pthread_mutex_lock(&trinco);
            printf("Total Kms percorridos pela frota: %d\n", total_kms);
            pthread_mutex_unlock(&trinco);
        }
        else if (strcmp(cmd, "frota") == 0)
        {
            pthread_mutex_lock(&trinco);
            printf("--- Estado da Frota ---\n");
            int ativos = 0;
            for(int k=0; k<t2_data.total_servicos; k++) {
                if(tab_servicos[k].estado == 1) { // Só os que estão a andar
                    printf("Veiculo %d (Cliente: %s) -> %d%% completado\n", 
                        tab_servicos[k].id, tab_servicos[k].nome_cliente, tab_servicos[k].percorrido);
                    ativos++;
                }
            }
            if(ativos==0) printf("Nenhum veiculo a circular.\n");
            pthread_mutex_unlock(&trinco);
        }
        else if (strcmp(cmd, "cancelar") == 0)
        {
            int id_alvo;
            // Lê o ID a seguir ao comando
            if (scanf("%d", &id_alvo) == 1) 
            {
                pthread_mutex_lock(&trinco);
                int count = 0;
                for(int k=0; k<t2_data.total_servicos; k++) {
                    // ID=0 apaga todos; ID especifico apaga so esse
                    if((id_alvo == 0 || tab_servicos[k].id == id_alvo) && tab_servicos[k].estado != 2) {
                        if(tab_servicos[k].estado == 1) kill(tab_servicos[k].pid_veiculo, SIGUSR1);
                        tab_servicos[k].estado = 2; // Cancelado
                        count++;
                    }
                }
                printf("Cancelados %d servicos.\n", count);
                pthread_mutex_unlock(&trinco);
            } else {
                 printf("Erro: Use 'cancelar <id>'\n");
                 // Limpar buffer do scanf
                 int c; while ((c = getchar()) != '\n' && c != EOF);
            }
        }
        else if (strcmp(cmd, "utiliz") == 0)
        {
            pthread_mutex_lock(&trinco);
            printf("Clientes registados (%d):\n", t2_data.total_clientes);
            for (i = 0; i < MAX_CLI; i++)
            {
                if (tab_clientes[i].ativo)
                    printf("- %s\n", tab_clientes[i].username);
            }
            pthread_mutex_unlock(&trinco);
        }
        else if (strcmp(cmd, "listar") == 0)
        {
            pthread_mutex_lock(&trinco);

            printf("Serviços\n", t2_data.total_servicos);
            for (int k = 0; k < t2_data.total_servicos; k++){
                if (tab_servicos[k].estado==0)
                    printf("ID: %d | User: %s | Estado: %d\n", tab_servicos[k].id, tab_servicos[k].nome_cliente, tab_servicos[k].estado);
            }
                
            pthread_mutex_unlock(&trinco);
        }
        else if (strcmp(cmd, "terminar") == 0)
        {
            // avisar clientes que vão sair
            for (int i = 0; i < t2_data.total_clientes; i++)
            {
                if (tab_clientes[i].ativo)
                {
                    int fd_cli;
                    sprintf(mensagem, "A desconectar...servidor terminou sessao.\n");
                    fd_cli = open(tab_clientes[i].fifo_cliente, O_WRONLY);
                    write(fd_cli, mensagem, strlen(mensagem) + 1);
                    close(fd_cli);
                }
                else
                {
                    break;
                }
            }
            break;
        }
        else
        {
            printf("Comando desconhecido.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }
    printf("A encerrar servidor...\n");

    close(fd_s);
    unlink(FIFO_SERV);
    exit(0);
}