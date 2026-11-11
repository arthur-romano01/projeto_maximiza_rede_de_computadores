CC = gcc
CFLAGS = -O3 -Wall -Wextra

all: servidor cliente
servidor: servidor.c
	$(CC) $(CFLAGS) -pthread servidor.c -o servidor
cliente: cliente.c
	$(CC) $(CFLAGS) cliente.c -o cliente
clean:
	rm -f servidor cliente