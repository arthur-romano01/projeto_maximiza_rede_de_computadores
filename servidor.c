
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_CLIENTS 10
#define BUFFER_SIZE 65536

// Estrutura para armazenar os dados de cada cliente.
typedef struct {
    SOCKET socket;
    int id;
    long long total_bytes;
    int error;
} ClientInfo;

// Calcula o tempo decorrido em segundos.
double elapsed_time(LARGE_INTEGER start,
                    LARGE_INTEGER end,
                    LARGE_INTEGER frequency) {

    return (double)(end.QuadPart - start.QuadPart)
           / frequency.QuadPart;
}

// Thread responsavel por receber os dados de um cliente.
DWORD WINAPI receive_data(LPVOID arg) {

    ClientInfo *client = (ClientInfo *)arg;

    char buffer[BUFFER_SIZE];

    int received;

    client->total_bytes = 0;
    client->error = 0;

    // Receber dados continuamente.
    while (1) {

        received = recv(
            client->socket,
            buffer,
            sizeof(buffer),
            0
        );

        if (received > 0) {

            // Contabilizar somente os bytes efetivamente recebidos.
            client->total_bytes += received;

        }
        else if (received == 0) {

            // Cliente encerrou a transmissao.
            break;

        }
        else {

            int error_code = WSAGetLastError();

            if (error_code == WSAEINTR) {
                continue;
            }

            client->error = error_code;
            break;
        }
    }

    closesocket(client->socket);

    return 0;
}

int main(int argc, char *argv[]) {

    // Verificar argumentos.
    if (argc != 3) {

        printf("Uso: %s <porta> <numero_clientes>\n", argv[0]);

        return 1;
    }

    int port = atoi(argv[1]);
    int num_clients = atoi(argv[2]);

    if (port < 1 || port > 65535 ||
        num_clients < 1 || num_clients > MAX_CLIENTS) {

        printf("Parametros invalidos.\n");

        return 1;
    }

    // Inicializar Winsock.
    WSADATA wsa_data;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {

        printf("Erro ao inicializar Winsock.\n");

        return 1;
    }

    // Criar socket do servidor.
    SOCKET server_socket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (server_socket == INVALID_SOCKET) {

        printf("Erro ao criar socket: %d\n", WSAGetLastError());

        WSACleanup();

        return 1;
    }

    // Configurar endereco do servidor.
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((u_short)port);

    // Associar socket a porta.
    if (bind(server_socket,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == SOCKET_ERROR) {

        printf("Erro no bind: %d\n", WSAGetLastError());

        closesocket(server_socket);
        WSACleanup();

        return 1;
    }

    // Colocar servidor em modo de escuta.
    if (listen(server_socket, MAX_CLIENTS) == SOCKET_ERROR) {

        printf("Erro no listen: %d\n", WSAGetLastError());

        closesocket(server_socket);
        WSACleanup();

        return 1;
    }

    printf("Porta: %d\n", port);
    printf("Clientes esperados: %d\n", num_clients);

    printf("\nAguardando conexoes...\n");

    // Estruturas dos clientes e das threads.
    ClientInfo clients[MAX_CLIENTS];

    HANDLE threads[MAX_CLIENTS];

    // Aceitar todos os clientes antes do teste.
    for (int i = 0; i < num_clients; i++) {

        struct sockaddr_in client_addr;

        int addr_len = sizeof(client_addr);

        SOCKET client_socket = accept(
            server_socket,
            (struct sockaddr *)&client_addr,
            &addr_len
        );

        if (client_socket == INVALID_SOCKET) {

            printf("Erro no accept: %d\n", WSAGetLastError());

            closesocket(server_socket);
            WSACleanup();

            return 1;
        }

        clients[i].socket = client_socket;
        clients[i].id = i + 1;
        clients[i].total_bytes = 0;
        clients[i].error = 0;

        printf("Cliente %d conectado: %s\n",
               i + 1,
               inet_ntoa(client_addr.sin_addr));
    }

    printf("\nTodos os clientes conectados!\n");

    // Criar uma thread para cada cliente.
    for (int i = 0; i < num_clients; i++) {

        threads[i] = CreateThread(
            NULL,
            0,
            receive_data,
            &clients[i],
            0,
            NULL
        );

        if (threads[i] == NULL) {

            printf("Erro ao criar thread: %lu\n", GetLastError());

            // Encerrar conexoes e aguardar threads ja criadas.
            for (int j = 0; j < i; j++) {
                shutdown(clients[j].socket, SD_BOTH);
                WaitForSingleObject(threads[j], INFINITE);
                CloseHandle(threads[j]);
            }

            for (int j = i; j < num_clients; j++) {
                closesocket(clients[j].socket);
            }

            closesocket(server_socket);
            WSACleanup();

            return 1;
        }
    }

    // Preparar sinal de inicio.
    char start_signal = 'G';

    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER end;

    QueryPerformanceFrequency(&frequency);

    printf("\nIniciando transmissao...\n");

    // Iniciar cronometro antes de liberar os clientes.
    QueryPerformanceCounter(&start);

    // Liberar todos os clientes.
    for (int i = 0; i < num_clients; i++) {

        int sent = send(
            clients[i].socket,
            &start_signal,
            1,
            0
        );

        if (sent != 1) {

            printf("Erro ao liberar cliente %d: %d\n",
                   i + 1, WSAGetLastError());

            shutdown(clients[i].socket, SD_BOTH);
        }
    }

    // Aguardar todas as threads terminarem.
    for (int i = 0; i < num_clients; i++) {

        WaitForSingleObject(threads[i], INFINITE);

        CloseHandle(threads[i]);
    }

    // Finalizar medicao.
    QueryPerformanceCounter(&end);

    double duration = elapsed_time(start, end, frequency);

    // Somar bytes recebidos de todos os clientes.
    long long total_bytes = 0;

    printf("RESULTADOS:\n");

    for (int i = 0; i < num_clients; i++) {

        printf("Cliente %d: %lld bytes",
               clients[i].id,
               clients[i].total_bytes);

        if (clients[i].error != 0) {
            printf(" (erro: %d)", clients[i].error);
        }

        printf("\n");

        total_bytes += clients[i].total_bytes;
    }

    // Calcular throughput agregado em Mbps.
    double throughput =
        (total_bytes * 8.0) / (duration * 1000000.0);

    printf("Clientes simultaneos: %d\n", num_clients);

    printf("Total recebido: %lld bytes\n", total_bytes);

    printf("Tempo medido: %.4f segundos\n", duration);

    printf("Throughput agregado: %.2f Mbps\n", throughput);

    // Liberar recursos.
    closesocket(server_socket);

    WSACleanup();

    return 0;
}