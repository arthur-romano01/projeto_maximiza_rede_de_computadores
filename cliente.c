
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER_SIZE (1024 * 1024)

// Calcula o tempo decorrido em segundos.
double elapsed_time(LARGE_INTEGER start,
                    LARGE_INTEGER end,
                    LARGE_INTEGER frequency) {

    return (double)(end.QuadPart - start.QuadPart)
           / frequency.QuadPart;
}

int main(int argc, char *argv[]) {

    // Verificar argumentos.
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

    // Inicializar Winsock.
    WSADATA wsa_data;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        printf("Erro ao inicializar Winsock.\n");
        return 1;
    }

    // Criar socket TCP.
    SOCKET client_socket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (client_socket == INVALID_SOCKET) {
        printf("Erro ao criar socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Configurar endereco do servidor.
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((u_short)port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) != 1) {

        printf("Endereco IP invalido.\n");

        closesocket(client_socket);
        WSACleanup();

        return 1;
    }

    // Conectar ao servidor.
    if (connect(client_socket,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == SOCKET_ERROR) {

        printf("Erro na conexao: %d\n", WSAGetLastError());

        closesocket(client_socket);
        WSACleanup();

        return 1;
    }

    printf("Conectado ao servidor!\n");
    printf("Aguardando inicio do teste...\n");

    // Aguardar sinal do servidor.
    char start_signal;

    int signal_received;

    do {
        signal_received = recv(
            client_socket,
            &start_signal,
            1,
            0
        );

    } while (signal_received == SOCKET_ERROR &&
             WSAGetLastError() == WSAEINTR);

    if (signal_received != 1 || start_signal != 'G') {

        printf("Nao foi possivel iniciar o teste.\n");

        closesocket(client_socket);
        WSACleanup();

        return 1;
    }

    // Alocar buffer de transmissao.
    char *buffer = (char *)malloc(block_size);

    if (buffer == NULL) {

        printf("Erro ao alocar memoria.\n");

        closesocket(client_socket);
        WSACleanup();

        return 1;
    }

    memset(buffer, 'A', block_size);

    printf("Iniciando transmissao...\n");
    printf("Duracao: %d segundos\n", duration);
    printf("Tamanho do bloco: %d bytes\n", block_size);

    // Inicializar medicao de tempo.
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER now;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    long long total_bytes = 0;

    // Transmitir continuamente.
    while (1) {

        QueryPerformanceCounter(&now);

        double elapsed = elapsed_time(start, now, frequency);

        if (elapsed >= duration) {
            break;
        }

        int sent_total = 0;

        // Garantir envio do bloco completo.
        while (sent_total < block_size) {

            int sent = send(
                client_socket,
                buffer + sent_total,
                block_size - sent_total,
                0
            );

            if (sent > 0) {

                sent_total += sent;
                total_bytes += sent;

            } else {

                printf("Erro no envio: %d\n", WSAGetLastError());

                free(buffer);
                closesocket(client_socket);
                WSACleanup();

                return 1;
            }
        }
    }

    // Encerrar apenas o envio.
    shutdown(client_socket, SD_SEND);

    // Liberar recursos.
    free(buffer);
    closesocket(client_socket);
    WSACleanup();

    printf("\n===== CLIENTE FINALIZADO =====\n");
    printf("Bytes enviados: %lld\n", total_bytes);

    return 0;
}