
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE (1024 * 1024)

// Calcula o tempo decorrido em segundos.
double elapsed_time(struct timespec start,
                    struct timespec end) {

    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(int argc, char *argv[]) {

    if (argc != 5) {
        printf("Uso: %s <IP> <porta> <duracao> <bloco_bytes>\n",
               argv[0]);
        return 1;
    }

    char *ip = argv[1];

    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int block_size = atoi(argv[4]);

    if (port < 1 || port > 65535 ||
        duration < 1 ||
        block_size < 1 ||
        block_size > MAX_BUFFER_SIZE) {

        printf("Parametros invalidos.\n");
        return 1;
    }

    // Criar socket TCP.
    int client_socket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (client_socket < 0) {
        perror("Erro ao criar socket");
        return 1;
    }

    // Configurar endereco do servidor.
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) != 1) {

        printf("Endereco IP invalido.\n");
        close(client_socket);
        return 1;
    }

    // Conectar ao servidor.
    if (connect(client_socket,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {

        perror("Erro na conexao");
        close(client_socket);
        return 1;
    }

    printf("Conectado ao servidor!\n");
    printf("Aguardando inicio do teste...\n");

    // Aguardar sinal do servidor.
    char start_signal;
    ssize_t signal_received;

    do {
        signal_received = recv(
            client_socket,
            &start_signal,
            1,
            0
        );

    } while (signal_received < 0 && errno == EINTR);

    if (signal_received != 1 || start_signal != 'G') {

        printf("Nao foi possivel iniciar o teste.\n");
        close(client_socket);
        return 1;
    }

    // Alocar buffer de transmissao.
    char *buffer = malloc(block_size);

    if (buffer == NULL) {

        perror("Erro ao alocar memoria");
        close(client_socket);
        return 1;
    }

    memset(buffer, 'A', block_size);

    printf("Iniciando transmissao...\n");
    printf("Duracao: %d segundos\n", duration);
    printf("Tamanho do bloco: %d bytes\n", block_size);

    // Inicializar medicao de tempo.
    struct timespec start, now;

    clock_gettime(CLOCK_MONOTONIC, &start);

    long long total_bytes = 0;

    // Transmitir continuamente.
    while (1) {

        clock_gettime(CLOCK_MONOTONIC, &now);

        double elapsed = elapsed_time(start, now);

        if (elapsed >= duration) {
            break;
        }

        int sent_total = 0;

        while (sent_total < block_size) {

            ssize_t sent = send(
                client_socket,
                buffer + sent_total,
                block_size - sent_total,
                MSG_NOSIGNAL
            );

            if (sent > 0) {

                sent_total += (int)sent;
                total_bytes += sent;

            } else if (sent < 0 && errno == EINTR) {

                continue;

            } else {

                perror("Erro no envio");

                free(buffer);
                close(client_socket);
                return 1;
            }
        }
    }

    // Informar que nao existem mais dados para enviar.
    shutdown(client_socket, SHUT_WR);

    // Liberar recursos.
    free(buffer);
    close(client_socket);

    printf("\n===== CLIENTE FINALIZADO =====\n");
    printf("Bytes enviados: %lld\n", total_bytes);

    return 0;
}
