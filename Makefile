
CC = gcc
CFLAGS = -O3 -Wall -Wextra

all: servidor cliente cliente-linux

# Servidor Windows (MinGW)
servidor:
	$(CC) $(CFLAGS) servidor.c -o servidor.exe -lws2_32

# Cliente Windows (MinGW)
cliente:
	$(CC) $(CFLAGS) cliente.c -o cliente.exe -lws2_32

# Cliente Linux
cliente-linux:
	$(CC) $(CFLAGS) cliente_linux.c -o cliente

clean:
	rm -f servidor.exe cliente.exe servidor cliente
