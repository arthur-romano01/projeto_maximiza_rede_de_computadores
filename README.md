# Projeto: Maximização do Throughput em Rede de Computadores

Projeto da disciplina de **Redes de Computadores (Unifesp)** para medir o **throughput agregado** de uma rede TCP quando vários clientes transmitem dados simultaneamente para um único servidor.

## Visão geral

O servidor (`servidor.c`) é escrito em C usando **Winsock2** (Windows). Ele:

1. Escuta em uma porta TCP informada pelo usuário;
2. Aguarda a conexão de **todos** os clientes esperados antes de iniciar o teste;
3. Cria uma **thread por cliente** para receber os dados em paralelo;
4. Envia um sinal de início (`'G'`) a todos os clientes e inicia o cronômetro (`QueryPerformanceCounter`);
5. Contabiliza os bytes recebidos de cada cliente até que todos encerrem a conexão;
6. Exibe o total recebido, o tempo medido e o **throughput agregado em Mbps**.

## Protocolo esperado do cliente

Para funcionar com o servidor, um cliente deve:

1. Conectar-se via TCP ao IP/porta do servidor;
2. Aguardar o recebimento de **1 byte** com o caractere `'G'` (sinal de início);
3. Enviar dados continuamente;
4. Encerrar a conexão (`shutdown`/`closesocket`) ao terminar a transmissão.

> O servidor considera a transmissão de um cliente finalizada quando `recv` retorna `0` (conexão encerrada) ou ocorre um erro.

## Compilação

**MinGW (gcc):**

```bash
gcc servidor.c -o servidor.exe -lws2_32
```

**MSVC (Developer Command Prompt):**

```bash
cl servidor.c
```

> No MSVC a biblioteca `Ws2_32.lib` é vinculada automaticamente via `#pragma comment`.

## Execução

```bash
servidor.exe <porta> <numero_clientes>
```

| Parâmetro         | Descrição                                   | Limites       |
|-------------------|---------------------------------------------|---------------|
| `porta`           | Porta TCP em que o servidor irá escutar     | 1 – 65535     |
| `numero_clientes` | Quantidade de clientes simultâneos esperada | 1 – 10        |

Exemplo:

```bash
servidor.exe 5000 4
```

> Pode ser necessário liberar a porta no Firewall do Windows para aceitar conexões de outras máquinas.

## Cálculo do throughput

```
throughput (Mbps) = (total_bytes × 8) / (tempo_em_segundos × 1.000.000)
```

O tempo é medido desde o envio do sinal de início até o término de todas as threads de recepção.

## Configurações

Definidas no início de `servidor.c`:

| Constante     | Valor   | Descrição                              |
|---------------|---------|----------------------------------------|
| `MAX_CLIENTS` | `10`    | Número máximo de clientes simultâneos  |
| `BUFFER_SIZE` | `65536` | Tamanho do buffer de recepção (bytes)  |

## Estrutura do repositório

```
.
├── servidor.c   # Servidor TCP multithread de medição de throughput
├── README.md
└── .gitignore
```
