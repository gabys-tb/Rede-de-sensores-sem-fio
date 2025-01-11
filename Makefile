# Makefile para compilar o cliente e o servidor em C

# Compilador
CC = gcc

# Diretórios
BIN_DIR = bin
INCLUDE_DIR = include
SRC_DIR = src

# Arquivos-fonte
CLIENT_SRC = client.c $(SRC_DIR)/clientUtils.c
SERVER_SRC = server.c

# Binários
CLIENT_BIN = $(BIN_DIR)/client
SERVER_BIN = $(BIN_DIR)/server

# Flags de compilação
CFLAGS = -std=c11 -Wall -g -I$(INCLUDE_DIR)
LDFLAGS=-lpthread -lm  # Linker flags

# Regra padrão
all: $(CLIENT_BIN) $(SERVER_BIN)

# Regra para compilar o cliente
$(CLIENT_BIN): $(CLIENT_SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_BIN) $(LDFLAGS)

# Regra para compilar o servidor
$(SERVER_BIN): $(SERVER_SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_BIN) $(LDFLAGS)

# Limpeza dos binários
clean:
	rm -rf $(BIN_DIR)
