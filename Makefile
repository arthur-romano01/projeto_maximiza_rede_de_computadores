
CC = gcc
CFLAGS = -O3 -Wall -Wextra
LDLIBS = -lws2_32

all: servidor cliente cliente-linux

# Servidor Windows (MinGW)
servidor:
	$(CC) $(CFLAGS) servidor.c -o servidor.exe $(LDLIBS)

# Cliente Windows (MinGW)
cliente:
	$(CC) $(CFLAGS) cliente.c -o cliente.exe $(LDLIBS)

# Cliente Linux
cliente-linux:
	$(CC) $(CFLAGS) cliente_linux.c -o cliente

clean:
	rm -f servidor.exe cliente.exe servidor cliente
