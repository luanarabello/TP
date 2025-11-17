#include <stdio.h>
#include "veiculo.h"
#include "uteis.h"

typedef struct{
    char*username;
    pid_t pid_cli;
    Veiculo *automovel;
}Cliente

