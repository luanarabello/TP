#include "uteis.h"


// Função da thread para ler mensagens do servidor/veículo a qualquer momento
void *thread_leitura(void *arg) {
    char *nome_fifo = (char *)arg;
    int fd;
    char buffer[TAM_MAX];
    int n;

    fd = open(nome_fifo, O_RDWR); 
    if (fd == -1) {
        perror("Erro thread fifo");
        pthread_exit(NULL);
    }

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        n = read(fd, buffer, sizeof(buffer)-1);
        
        if (n > 0) {
            // Imprime o que chegou e repõe o prompt ">"
            printf("\n\n[MENSAGEM]: %s\n> ", buffer); 
            fflush(stdout);
        }
    }
    close(fd);
    return NULL;
}

int main(int argc, char *argv[])
{
    int fd_cli, fd_serv;
    char nome_fifo_cli[TAM_MAX]; 
    char cmd[TAM_MAX];    
    char buffer[TAM_MAX];
    Pedido p;
    pthread_t tid; // identificador da thread

    // 1. Verificações
    if (argc != 2) {
        printf("Uso: ./cliente <username>\n");
        exit(1);
    }

    if (access(FIFO_SERV, F_OK) != 0) {
        printf("ERRO: O controlador nao esta a correr\n");
        exit(1);
    }

    // criar FIFO do cliente
    memset(nome_fifo_cli, 0, sizeof(nome_fifo_cli));
    sprintf(nome_fifo_cli, FIFO_CLI_FMT, argv[1]); 

    // Se falhar porque já existe ignora e continua
    if (mkfifo(nome_fifo_cli, 0666) == -1) {
        if (errno != EEXIST) {
            perror("Erro ao criar FIFO");
            exit(1);
        }
    }

    //abrir servidor
    fd_serv = open(FIFO_SERV, O_WRONLY);
    if (fd_serv == -1) {
        fprintf(stderr, "\nO servidor não está a correr (Erro open)\n");
        unlink(nome_fifo_cli);
        exit(EXIT_FAILURE);
    }

    //PREENCHER E ENVIAR LOGIN
    p.tipo = REQ_LOGIN;
    strcpy(p.username, argv[1]);
    strcpy(p.fifo_cli, nome_fifo_cli); // manda o nome correto: ..._nomecli
    p.pid_cli = getpid();

    write(fd_serv, &p, sizeof(Pedido));

    //ler resposta do Login (bloqueante inicial)
    fd_cli = open(nome_fifo_cli, O_RDONLY);
    if (fd_cli == -1) {
        perror("Erro ao abrir FIFO cliente para leitura");
        exit(1);
    }

    memset(buffer, 0, sizeof(buffer));
    read(fd_cli, buffer, sizeof(buffer));
    
    if (strncmp(buffer, "LOGIN_OK", 8) != 0) {
        printf("[ERRO] Login recusado pelo servidor: %s\n", buffer);
        close(fd_cli);
        close(fd_serv);
        unlink(nome_fifo_cli);
        exit(1);
    }
    printf("\nLOGIN ACEITE! Ola %s.\n", argv[1]);
    
    // Fechar este fd para a thread abrir o dela à vontade
    close(fd_cli);

    // Lançar a thread para ouvir o veiculo/controlador
    if (pthread_create(&tid, NULL, thread_leitura, (void *)nome_fifo_cli) != 0) {
        perror("Erro criar thread");
        exit(1);
    }
    
    while(1) {
        //  sleep para as mensagens da thread não se misturarem com o menu 
        usleep(100000); 

        printf("\nOpcoes:\n");
        printf(" agendar   <hora> <local> <distancia>\n");
        printf(" consultar\n");
        printf(" sair\n");
        printf("> ");
        
        scanf("%s", cmd);

        if (strcmp(cmd, "sair") == 0) {
            break;
        }
        else if (strcmp(cmd, "agendar") == 0) {
            p.tipo = REQ_AGENDAR;
            if(scanf("%d %s %d", &p.hora, p.local_partida, &p.distancia) != 3) {
                printf(" Erro nos argumentos. Ex: agendar 10 Coimbra 50\n");
                // limpar buffer
                int c; while((c = getchar()) != '\n' && c != EOF);
                continue;
            }

            strcpy(p.username, argv[1]); 
            strcpy(p.fifo_cli, nome_fifo_cli);
            
            write(fd_serv, &p, sizeof(Pedido));
            
            // A resposta vai aparecer via thread, nao lemos aqui
        }
        else if (strcmp(cmd, "consultar") == 0) {
            p.tipo = REQ_CONSULTAR;
            strcpy(p.username, argv[1]);
            strcpy(p.fifo_cli, nome_fifo_cli);
            write(fd_serv, &p, sizeof(Pedido));
        }
        else if (strcmp(cmd, "cancelar") == 0) {
            p.tipo = REQ_CANCELAR;
            if(scanf("%d", &p.id_servico) != 1) {
                printf("Erro argumentos. Ex: cancelar 1\n");
                int c; while((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            strcpy(p.username, argv[1]);
            strcpy(p.fifo_cli, nome_fifo_cli);
            write(fd_serv, &p, sizeof(Pedido));
        }
        else {
            printf("Comando desconhecido: %s\n", cmd);
        }
    }

    close(fd_serv);
    unlink(nome_fifo_cli);
    return 0;
}