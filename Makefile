CC = gcc
CFLAGS = -Wall -Wextra -pthread -g

# Fontes (assumindo que estao na mesma pasta que o Makefile)
SRCS = controlador.c cliente.c veiculo.c uteis.c
OBJS = $(SRCS:.c=.o)

all: controlador cliente veiculo

controlador: controlador.o uteis.o
	$(CC) $(CFLAGS) -o controlador controlador.o uteis.o

cliente: cliente.o uteis.o
	$(CC) $(CFLAGS) -o cliente cliente.o uteis.o

veiculo: veiculo.o uteis.o
	$(CC) $(CFLAGS) -o veiculo veiculo.o uteis.o

# Regra para compilar .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o controlador cliente veiculo /tmp/fifo*