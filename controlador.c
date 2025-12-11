#include "uteis.h"
#include <pthread.h>

int fd_s;

void trataSig(int i)
{
    (void)i;
    fprintf(stderr, "\n[CONTROLADOR] A encerrar via sinal...\n");
    close(fd_s);
    unlink(FIFO_SERV);
    exit(EXIT_SUCCESS);
}

int utilizador_existe(char *user, Cliente *clientes, int total) {
    for (int i = 0; i < total; i++)
        if (clientes[i].ativo && strcmp(clientes[i].username, user) == 0) return 1;
    return 0;
}

int adiciona_cliente(char *user, Cliente *clientes, char *fifo, int *total) {
    if ((*total) >= MAX_CLI) {
        printf("[ERRO] Lista cheia\n");
        return 0;
    }
    strcpy(clientes[*total].username, user);
    strcpy(clientes[*total].fifo_cliente, fifo);
    clientes[*total].ativo = true;
    (*total)++;
    return 1;
}

void lancar_veiculo(Servico *s, char *fifo_cli) {
    int p[2];
    
    if (pipe(p) == -1) { perror("[ERRO] Pipe falhou"); return; }

    pid_t pid = fork();
    if (pid == -1) { perror("[ERRO] Fork falhou"); return; }

    if (pid == 0) { // --- FILHO ---
        close(p[0]);
        dup2(p[1], STDOUT_FILENO); 
        close(p[1]); 

        char str_id[10], str_dist[10];
        sprintf(str_id, "%d", s->id);
        sprintf(str_dist, "%d", s->dist_total);

        execl("./veiculo", "./veiculo", str_id, fifo_cli, str_dist, NULL);
        exit(1);
    } 
    else { // --- PAI ---
        close(p[1]);
        s->pipe_fd = p[0];
        s->pid_veiculo = pid;
        s->estado = 1; // Em Curso
        
        int flags = fcntl(s->pipe_fd, F_GETFL, 0);
        fcntl(s->pipe_fd, F_SETFL, flags | O_NONBLOCK);
        
        printf("[SISTEMA] Veiculo lançado (PID: %d)\n", pid);
    }
}

// --- THREAD NOVA: Monitoriza os veículos constantemente ---
void *thread_telemetria(void *arg) {
    TDATA_CLIENTES *ptd = (TDATA_CLIENTES *)arg;
    char buffer_veiculo[100];

    while(1) {
        for(int k=0; k < ptd->total_servicos; k++) {
            // Se o serviço está "Em Curso" (estado 1)
            if(ptd->lista_servicos[k].estado == 1) { 
                int n = read(ptd->lista_servicos[k].pipe_fd, buffer_veiculo, sizeof(buffer_veiculo)-1);
                if (n > 0) {
                    buffer_veiculo[n] = '\0';
                    // \r limpa a linha atual para não estragar o prompt do admin
                    printf("\r[VEICULO %d]: %sAdmin > ", ptd->lista_servicos[k].id, buffer_veiculo);
                    fflush(stdout);
                }
            }
        }
        usleep(100000); // Verifica a cada 0.1s
    }
    return NULL;
}

void *thread_clientes(void *arg)
{
    TDATA_CLIENTES *ptd = (TDATA_CLIENTES *)arg;
    Pedido p;
    int fd_cli;

    printf("[THREAD] Thread de clientes a correr...\n");

    while (1) {
        int n = read(fd_s, &p, sizeof(Pedido));
        if (n <= 0) { usleep(1000); continue; }

        if (p.tipo == REQ_LOGIN) {
            int existe = utilizador_existe(p.username, ptd->clientes, ptd->total_clientes);
            int verifica = 0;
            if (!existe) verifica = adiciona_cliente(p.username, ptd->clientes, p.fifo_cli, &ptd->total_clientes);
            
            fd_cli = open(p.fifo_cli, O_WRONLY);
            if(fd_cli != -1) {
                if(verifica) { write(fd_cli, "LOGIN_OK", 9); printf("[LOGIN] Novo: %s\n", p.username); }
                else { write(fd_cli, "LOGIN_ERRO", 11); }
                close(fd_cli);
            }
        }
        else if (p.tipo == REQ_AGENDAR) {
            char resposta[TAM_MAX];
            if (ptd->total_servicos < TAM_MAX) {
                Servico *novo = &ptd->lista_servicos[ptd->total_servicos];
                novo->id = ptd->total_servicos + 1;
                strcpy(novo->nome_cliente, p.username);
                novo->estado = 0; 
                novo->percorrido = 0;
                novo->inicio_servico.segundos = p.hora;
                novo->dist_total = p.distancia;
                strcpy(novo->local_partida, p.local_partida);

                lancar_veiculo(novo, p.fifo_cli);

                ptd->total_servicos++;
                sprintf(resposta, "Servico %d iniciado! Veiculo a caminho.", novo->id);
            } else {
                sprintf(resposta, "Erro: Frota cheia.");
            }
            
            fd_cli = open(p.fifo_cli, O_WRONLY);
            if(fd_cli != -1) { write(fd_cli, resposta, strlen(resposta)+1); close(fd_cli); }
        }
        else if (p.tipo == REQ_CONSULTAR) {
            char lista_completa[TAM_MAX * 5] = "";
            char linha[200];
            int encontrou = 0;

            sprintf(lista_completa, "\n--- Servicos de %s ---\n", p.username);
            for (int k = 0; k < ptd->total_servicos; k++) {
                if (strcmp(ptd->lista_servicos[k].nome_cliente, p.username) == 0) {
                    char *st_str = (ptd->lista_servicos[k].estado == 1) ? "EM CURSO" : "TERMINADO/CANCELADO";
                    sprintf(linha, "ID: %d | Origem: %s | Estado: %s\n", 
                            ptd->lista_servicos[k].id, ptd->lista_servicos[k].local_partida, st_str);
                    strcat(lista_completa, linha);
                    encontrou = 1;
                }
            }
            if (!encontrou) strcat(lista_completa, "Nenhum servico.\n");
            
            fd_cli = open(p.fifo_cli, O_WRONLY);
            if (fd_cli != -1) { write(fd_cli, lista_completa, strlen(lista_completa)+1); close(fd_cli); }
        }
        else if (p.tipo == REQ_CANCELAR) {
            char res[TAM_MAX];
            int cancelados = 0;
            for (int k = 0; k < ptd->total_servicos; k++) {
                Servico *s = &ptd->lista_servicos[k];
                if (strcmp(s->nome_cliente, p.username) == 0) {
                    if (p.id_servico == 0 || s->id == p.id_servico) {
                        if (s->estado == 1) { 
                            kill(s->pid_veiculo, SIGUSR1);
                            s->estado = 2; 
                            cancelados++;
                        }
                    }
                }
            }
            sprintf(res, "Cancelados: %d", cancelados);
            fd_cli = open(p.fifo_cli, O_WRONLY);
            if (fd_cli != -1) { write(fd_cli, res, strlen(res)+1); close(fd_cli); }
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    int i, nveiculos;
    char cmd[TAM_MAX];

    Cliente tab_clientes[MAX_CLI];
    Servico tab_servicos[TAM_MAX];

    pthread_t tid_cli, tid_tel;
    TDATA_CLIENTES t2_data; 

    memset(tab_clientes, 0, sizeof(tab_clientes));
    memset(tab_servicos, 0, sizeof(tab_servicos));

    if (signal(SIGINT, trataSig) == SIG_ERR) {
        perror("Erro sinal"); exit(1);
    }

    char *env = getenv("NVEICULOS");
    nveiculos = env ? atoi(env) : 5;

    strcpy(tab_clientes[0].username, "Amanda"); tab_clientes[0].ativo = true;
    strcpy(tab_clientes[1].username, "Jose");   tab_clientes[1].ativo = true;
    strcpy(tab_clientes[2].username, "Luis");   tab_clientes[2].ativo = true;

    t2_data.clientes = tab_clientes;
    t2_data.total_clientes = 3; 
    t2_data.lista_servicos = tab_servicos;
    t2_data.total_servicos = 0; 

    criaFifo(FIFO_SERV);

    // Lançar Threads
    pthread_create(&tid_cli, NULL, thread_clientes, (void *)&t2_data);
    
    // Nova Thread para Telemetria
    pthread_create(&tid_tel, NULL, thread_telemetria, (void *)&t2_data);

    printf("Controlador iniciado (NVeiculos: %d). Aguardando...\n", nveiculos);
    
    fd_s = open(FIFO_SERV, O_RDWR);
    if (fd_s == -1) { perror("Erro FIFO"); exit(1); }

    while (1)
    {
        // O printf tem \r para o output da thread não estragar o visual
        printf("\rAdmin > "); 
        fflush(stdout);
        
        scanf("%s", cmd); // Scanf bloqueia aqui, mas as threads continuam a correr!

        if (strcmp(cmd, "utiliz") == 0) {
            printf("Clientes registados (%d):\n", t2_data.total_clientes);
            for (i = 0; i < MAX_CLI; i++) {
                if (tab_clientes[i].ativo)
                    printf("- %s\n", tab_clientes[i].username);
            }
        }
        else if (strcmp(cmd, "listar") == 0) {
             printf("--- Serviços (%d) ---\n", t2_data.total_servicos);
             for(int k=0; k < t2_data.total_servicos; k++)
                printf("ID: %d | User: %s | Estado: %d\n", tab_servicos[k].id, tab_servicos[k].nome_cliente, tab_servicos[k].estado);
        }
        else if (strcmp(cmd, "terminar") == 0) {
            kill(0, SIGINT);
            break;
        }
        else {
            printf("Comando desconhecido.\n");
        }
    }
    
    close(fd_s);
    unlink(FIFO_SERV);
    exit(0);
}