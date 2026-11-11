# Projeto: Maximização do Throughput em Rede de Computadores

Projeto da disciplina de **Redes de Computadores (UNIFESP)** para medir o **throughput agregado** de uma rede TCP quando vários clientes transmitem dados simultaneamente para um único servidor.

## Visão geral

O projeto é composto por dois programas desenvolvidos em C utilizando **Winsock2 (Windows)**: um servidor e múltiplos clientes.

### Servidor (`servidor.c`)

O servidor:

1. Escuta em uma porta TCP informada pelo usuário;
2. Aguarda a conexão de **todos** os clientes esperados antes de iniciar o teste;
3. Cria uma **thread por cliente** para receber os dados em paralelo;
4. Envia um sinal de início (`'G'`) a todos os clientes e inicia o cronômetro (`QueryPerformanceCounter`);
5. Contabiliza os bytes recebidos de cada cliente até que todos encerrem a conexão;
6. Exibe o total recebido, o tempo medido e o **throughput agregado em Mbps**.

### Cliente (`cliente.c`)

O cliente é responsável por estabelecer uma conexão TCP com o servidor e transmitir dados durante um período determinado.

Seu funcionamento ocorre nas seguintes etapas:

1. Recebe como parâmetros o endereço IP do servidor, a porta TCP, a duração do teste e o tamanho dos blocos de dados;
2. Inicializa a biblioteca Winsock2 e cria um socket TCP;
3. Configura o endereço IP e a porta do servidor;
4. Estabelece a conexão TCP com o servidor;
5. Aguarda o recebimento de um byte contendo o caractere `'G'`, enviado pelo servidor como sinal de início do teste;
6. Aloca um buffer de memória com o tamanho de bloco especificado, preenchido com o caractere `'A'`;
7. Inicia a medição do tempo com `QueryPerformanceCounter` e transmite blocos de dados continuamente até atingir a duração configurada;
8. Garante que cada bloco seja enviado completamente, contabilizando a quantidade total de bytes transmitidos;
9. Encerra o envio de dados utilizando `shutdown()` e libera os recursos utilizados.

O cliente utiliza `QueryPerformanceCounter` para controlar a duração da transmissão e contabiliza os bytes enviados, permitindo comparar o volume transmitido com os dados contabilizados pelo servidor.

## Protocolo de comunicação

A comunicação entre cliente e servidor segue o seguinte fluxo:

1. O servidor inicia e aguarda as conexões dos clientes;
2. Cada cliente estabelece uma conexão TCP com o servidor;
3. O servidor aguarda a conexão de todos os clientes esperados;
4. O servidor envia o sinal de início (`'G'`) para cada cliente;
5. Os clientes iniciam a transmissão contínua de dados;
6. Ao atingir a duração configurada, cada cliente encerra seu envio;
7. O servidor identifica o encerramento de cada transmissão e calcula o throughput agregado.

> O servidor considera a transmissão de um cliente finalizada quando `recv()` retorna `0` (conexão encerrada) ou ocorre um erro.

## Compilação

### MinGW (GCC)

**Servidor:**

```bash
gcc servidor.c -o servidor.exe -lws2_32
```

**Cliente:**

```bash
gcc cliente.c -o cliente.exe -lws2_32
```

### MSVC (Developer Command Prompt)

**Servidor:**

```bash
cl servidor.c
```

**Cliente:**

```bash
cl cliente.c
```

> No MSVC, a biblioteca `Ws2_32.lib` é vinculada automaticamente via `#pragma comment(lib, "Ws2_32.lib")`.

## Execução

### Servidor

```bash
servidor.exe <porta> <numero_clientes>
```

| Parâmetro | Descrição | Limites |
|---|---|---|
| `porta` | Porta TCP em que o servidor irá escutar | 1 – 65535 |
| `numero_clientes` | Quantidade de clientes simultâneos esperada | 1 – 10 |

Exemplo:

```bash
servidor.exe 5000 4
```

### Cliente

```bash
cliente.exe <IP> <porta> <duracao> <bloco_bytes>
```

| Parâmetro | Descrição | Limites |
|---|---|---|
| `IP` | Endereço IPv4 do servidor | Endereço IPv4 válido |
| `porta` | Porta TCP utilizada pelo servidor | 1 – 65535 |
| `duracao` | Tempo de transmissão em segundos | Maior ou igual a 1 |
| `bloco_bytes` | Tamanho de cada bloco de dados transmitido, em bytes | 1 – 1.048.576 |

Exemplo:

```bash
cliente.exe 192.168.1.100 5000 10 65536
```

Nesse exemplo, o cliente se conecta ao servidor no endereço `192.168.1.100`, utilizando a porta `5000`, e transmite dados durante 10 segundos, em blocos de 65.536 bytes (64 KiB).

> O endereço IP deve corresponder à máquina onde o servidor está sendo executado. Caso cliente e servidor estejam em máquinas diferentes, pode ser necessário liberar a porta no Firewall do Windows.

> O número de clientes iniciados deve corresponder ao número informado na execução do servidor.

## Cálculo do throughput

O servidor calcula o throughput agregado considerando a quantidade total de bytes recebidos de todos os clientes e o tempo total da medição.

```text
throughput (Mbps) = (total_bytes × 8) / (tempo_em_segundos × 1.000.000)
```

O tempo é medido desde o envio do sinal de início até o término de todas as threads de recepção.

## Configurações

### Servidor (`servidor.c`)

Definidas no início do código:

| Constante | Valor | Descrição |
|---|---|---|
| `MAX_CLIENTS` | `10` | Número máximo de clientes simultâneos |
| `BUFFER_SIZE` | `65536` | Tamanho do buffer de recepção (bytes) |

### Cliente (`cliente.c`)

Definida no início do código:

| Constante | Valor | Descrição |
|---|---|---|
| `MAX_BUFFER_SIZE` | `1048576` | Tamanho máximo permitido para o buffer de transmissão (1 MiB) |

O tamanho efetivo do bloco transmitido é definido pelo parâmetro `bloco_bytes` durante a execução.

## Estrutura do repositório

```text
.
├── cliente.c       # Cliente TCP para transmissão contínua de dados
├── servidor.c      # Servidor TCP multithread de medição de throughput
├── README.md
└── .gitignore
```