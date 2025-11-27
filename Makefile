#makefile

# Compiler and linker
CC=gcc

# Flags for compiler
CC_FLAGS=-g         \
         -Iinclude  \
         -Wall      \
         -Wextra    \

SCRDIR=TP
CLIENTE=cliente
VEICULO=veiculo
CONTROLADOR=controlador

# Object files -> substitui .c por .o e armazena na var OBJ
OBJ_UTIL=$(SCRDIR)/uteis.o
OBJ_CLI=$(SCRDIR)/cliente.o
OBJ_CONTROL=$(SCRDIR)/controlador.o
OBJ_VEIC=$(SCRDIR)/veiculo.o

#
# Compilation and linking
#
all: $(CONTROLADOR) $(CLIENTE) $(VEICULO)

$(CONTROLADOR): $(OBJ_CONTROL) $(OBJ_UTIL)
	$(CC) -o $@ $^

$(CLIENTE): $(OBJ_CLI) $(OBJ_UTIL)
	$(CC) -o $@ $^

$(VEICULO): $(OBJ_VEIC) $(OBJ_UTIL)
	$(CC) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRCDIR)/*.o $(CLIENTE) $(CONTROLADOR) $(VEICULO)

.PHONY: all clean

