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
.\servidor.exe <porta> <numero_clientes>
```

| Parâmetro | Descrição | Limites |
|---|---|---|
| `porta` | Porta TCP em que o servidor irá escutar | 1 – 65535 |
| `numero_clientes` | Quantidade de clientes simultâneos esperada | 1 – 10 |

Exemplo:

```bash
.\servidor.exe 5000 4
```

### Cliente

```bash
.\cliente.exe <IP> <porta> <duracao> <bloco_bytes>
```

| Parâmetro | Descrição | Limites |
|---|---|---|
| `IP` | Endereço IPv4 do servidor | Endereço IPv4 válido (ex: `127.0.0.1`) |
| `porta` | Porta TCP utilizada pelo servidor | 1 – 65535 |
| `duracao` | Tempo de transmissão em segundos | Maior ou igual a 1 |
| `bloco_bytes` | Tamanho de cada bloco de dados transmitido, em bytes | 1 – 1.048.576 |

Exemplo:

```bash
.\cliente.exe 127.0.0.1 5000 10 65536
```

Nesse exemplo, o cliente se conecta ao servidor no endereço local `127.0.0.1`, utilizando a porta `5000`, e transmite dados durante 10 segundos, em blocos de 65.536 bytes (64 KiB).

> **Atenção:** O número de clientes iniciados nos terminais deve corresponder exatamente ao número informado na execução do servidor para que o teste seja disparado.

## Cálculo do throughput

O servidor calcula o throughput agregado considerando a quantidade total de bytes recebidos de todos os clientes e o tempo total da medição:

```text
throughput (Mbps) = (total_bytes × 8) / (tempo_em_segundos × 1.000.000)
```

O tempo é medido desde o envio do sinal de início até o término de todas as threads de recepção.

## Configurações

### Servidor (`servidor.c`)

| Constante | Valor | Descrição |
|---|---|---|
| `MAX_CLIENTS` | `10` | Número máximo de clientes simultâneos |
| `BUFFER_SIZE` | `65536` | Tamanho do buffer de recepção (bytes) |

### Cliente (`cliente.c`)

| Constante | Valor | Descrição |
|---|---|---|
| `MAX_BUFFER_SIZE` | `1048576` | Tamanho máximo permitido para o buffer de transmissão (1 MiB) |

---

## Resultados dos Testes (1 a 10 Clientes) - Arthur

Os testes foram realizados em ambiente local (interface de *loopback* `127.0.0.1`), com duração configurada de **10 segundos** e blocos de **65.536 bytes (64 KiB)** por cliente.

### Tabela Comparativa de Desempenho

| Clientes Simultâneos | Duração Medida (s) | Total Recebido (Bytes) | Throughput Agregado (Mbps) | Throughput Agregado (Gbps) |
|:---:|:---:|:---:|:---:|:---:|
| **1** | 10.0024 s | 25.818.955.776 | 20.650,31 Mbps | ~20,65 Gbps |
| **2** | 10.0020 s | 52.880.736.256 | 42.296,12 Mbps | ~42,30 Gbps |
| **3** | 10.0024 s | 52.320.665.600 | 41.846,54 Mbps | ~41,85 Gbps |
| **4** | 10.0031 s | 71.237.763.072 | 56.972,30 Mbps | ~56,97 Gbps |
| **5** | 10.0067 s | 81.604.706.304 | 65.240,13 Mbps | ~65,24 Gbps |
| **6** | 10.0094 s | 48.338.829.312 | 38.634,90 Mbps | ~38,63 Gbps |
| **7** | 10.0166 s | 80.332.587.008 | 64.159,66 Mbps | ~64,16 Gbps |
| **8** | 10.0162 s | 48.026.091.520 | 38.358,86 Mbps | ~38,36 Gbps |
| **9** | 10.0140 s | 46.911.848.448 | 37.477,16 Mbps | ~37,48 Gbps |
| **10** | 10.0171 s | 90.278.723.584 | 72.099,59 Mbps | ~72,10 Gbps |

---

### Detalhamento por Teste

#### 1 Cliente
* **Dados por cliente:**
  * Cliente 1: 25.818.955.776 bytes
* **Total recebido:** 25.818.955.776 bytes (~25,82 GB)
* **Tempo:** 10.0024 s
* **Throughput:** **20.650,31 Mbps** (~20,65 Gbps)

<details>
<summary>Visualizar Captura de Tela (1 Cliente)</summary>

![Servidor - 1 Cliente](prints/Testes%20parte%201/Servidor_1_cliente.png)
![Cliente - 1 Cliente](prints/Testes%20parte%201/teste_1_cliente.png)
</details>

---

#### 2 Clientes
* **Dados por cliente:**
  * Cliente 1: 26.610.434.048 bytes
  * Cliente 2: 26.270.302.208 bytes
* **Total recebido:** 52.880.736.256 bytes (~52,88 GB)
* **Tempo:** 10.0020 s
* **Throughput:** **42.296,12 Mbps** (~42,30 Gbps)

<details>
<summary>Visualizar Captura de Tela (2 Clientes)</summary>

![Servidor - 2 Clientes](prints/Testes%20parte%201/Servidor_2_clientes.png)
![Cliente - 2 Clientes](prints/Testes%20parte%201/teste_2_clientes.png)
</details>

---

#### 3 Clientes
* **Dados por cliente:**
  * Cliente 1: 16.883.318.784 bytes
  * Cliente 2: 17.090.347.008 bytes
  * Cliente 3: 18.346.999.808 bytes
* **Total recebido:** 52.320.665.600 bytes (~52,32 GB)
* **Tempo:** 10.0024 s
* **Throughput:** **41.846,54 Mbps** (~41,85 Gbps)

<details>
<summary>Visualizar Captura de Tela (3 Clientes)</summary>

![Servidor - 3 Clientes](prints/Testes%20parte%201/Servidor_3_cliente.png)
![Cliente - 3 Clientes](prints/Testes%20parte%201/teste_3_cliente.png)
</details>

---

#### 4 Clientes
* **Dados por cliente:**
  * Cliente 1: 17.415.733.248 bytes
  * Cliente 2: 17.669.488.640 bytes
  * Cliente 3: 17.825.005.568 bytes
  * Cliente 4: 18.327.535.616 bytes
* **Total recebido:** 71.237.763.072 bytes (~71,24 GB)
* **Tempo:** 10.0031 s
* **Throughput:** **56.972,30 Mbps** (~56,97 Gbps)

<details>
<summary>Visualizar Captura de Tela (4 Clientes)</summary>

![Servidor - 4 Clientes](prints/Testes%20parte%201/Servidor_4_clientes.png)
![Cliente - 4 Clientes](prints/Testes%20parte%201/teste_4_clientes.png)
</details>

---

#### 5 Clientes
* **Dados por cliente:**
  * Cliente 1: 16.883.974.144 bytes
  * Cliente 2: 16.245.653.504 bytes
  * Cliente 3: 16.251.813.888 bytes
  * Cliente 4: 15.959.523.328 bytes
  * Cliente 5: 16.263.741.440 bytes
* **Total recebido:** 81.604.706.304 bytes (~81,60 GB)
* **Tempo:** 10.0067 s
* **Throughput:** **65.240,13 Mbps** (~65,24 Gbps)

<details>
<summary>Visualizar Captura de Tela (5 Clientes)</summary>

![Servidor - 5 Clientes](prints/Testes%20parte%201/Servidor_5_clientes.png)
![Cliente - 5 Clientes](prints/Testes%20parte%201/teste_5_clientes.png)
</details>

---

#### 6 Clientes
* **Dados por cliente:**
  * Cliente 1: 8.018.329.600 bytes
  * Cliente 2: 8.251.310.080 bytes
  * Cliente 3: 7.796.162.560 bytes
  * Cliente 4: 7.987.003.392 bytes
  * Cliente 5: 8.385.265.664 bytes
  * Cliente 6: 7.900.758.016 bytes
* **Total recebido:** 48.338.829.312 bytes (~48,34 GB)
* **Tempo:** 10.0094 s
* **Throughput:** **38.634,90 Mbps** (~38,63 Gbps)

<details>
<summary>Visualizar Captura de Tela (6 Clientes)</summary>

![Servidor - 6 Clientes](prints/Testes%20parte%201/Servidor_6_clientes.png)
![Cliente - 6 Clientes](prints/Testes%20parte%201/teste_6_cliente.png)
</details>

---

#### 7 Clientes
* **Dados por cliente:**
  * Cliente 1: 11.157.438.464 bytes
  * Cliente 2: 11.036.262.400 bytes
  * Cliente 3: 11.092.230.144 bytes
  * Cliente 4: 10.934.353.920 bytes
  * Cliente 5: 11.039.670.272 bytes
  * Cliente 6: 11.038.097.408 bytes
  * Cliente 7: 14.034.534.400 bytes
* **Total recebido:** 80.332.587.008 bytes (~80,33 GB)
* **Tempo:** 10.0166 s
* **Throughput:** **64.159,66 Mbps** (~64,16 Gbps)

<details>
<summary>Visualizar Captura de Tela (7 Clientes)</summary>

![Servidor - 7 Clientes](prints/Testes%20parte%201/Servidor_7_clientes.png)
![Cliente - 7 Clientes](prints/Testes%20parte%201/teste_7_clientes.png)
</details>

---

#### 8 Clientes
* **Dados por cliente:**
  * Cliente 1: 5.691.277.312 bytes
  * Cliente 2: 5.884.149.760 bytes
  * Cliente 3: 5.630.590.976 bytes
  * Cliente 4: 5.779.095.552 bytes
  * Cliente 5: 5.705.891.840 bytes
  * Cliente 6: 6.301.614.080 bytes
  * Cliente 7: 6.705.577.984 bytes
  * Cliente 8: 6.327.894.016 bytes
* **Total recebido:** 48.026.091.520 bytes (~48,03 GB)
* **Tempo:** 10.0162 s
* **Throughput:** **38.358,86 Mbps** (~38,36 Gbps)

<details>
<summary>Visualizar Captura de Tela (8 Clientes)</summary>

![Servidor - 8 Clientes](prints/Testes%20parte%201/Servidor_8_clientes.png)
![Cliente - 8 Clientes](prints/Testes%20parte%201/teste_8_clientes.png)
</details>

---

#### 9 Clientes
* **Dados por cliente:**
  * Cliente 1: 5.095.948.288 bytes
  * Cliente 2: 5.257.101.312 bytes
  * Cliente 3: 4.696.506.368 bytes
  * Cliente 4: 4.807.917.568 bytes
  * Cliente 5: 4.959.633.408 bytes
  * Cliente 6: 4.959.502.336 bytes
  * Cliente 7: 5.635.375.104 bytes
  * Cliente 8: 5.696.061.440 bytes
  * Cliente 9: 5.803.802.624 bytes
* **Total recebido:** 46.911.848.448 bytes (~46,91 GB)
* **Tempo:** 10.0140 s
* **Throughput:** **37.477,16 Mbps** (~37,48 Gbps)

<details>
<summary>Visualizar Captura de Tela (9 Clientes)</summary>

![Servidor - 9 Clientes](prints/Testes%20parte%201/Servidor_9_clientes.png)
![Cliente - 9 Clientes](prints/Testes%20parte%201/teste_9_clientes.png)
</details>

---

#### 10 Clientes
* **Dados por cliente:**
  * Cliente 1: 7.885.946.880 bytes
  * Cliente 2: 8.056.078.336 bytes
  * Cliente 3: 7.692.943.360 bytes
  * Cliente 4: 8.288.534.528 bytes
  * Cliente 5: 8.039.432.192 bytes
  * Cliente 6: 7.999.455.232 bytes
  * Cliente 7: 10.776.084.480 bytes
  * Cliente 8: 11.142.627.328 bytes
  * Cliente 9: 10.102.112.256 bytes
  * Cliente 10: 10.295.508.992 bytes
* **Total recebido:** 90.278.723.584 bytes (~90,28 GB)
* **Tempo:** 10.0171 s
* **Throughput:** **72.099,59 Mbps** (~72,10 Gbps)

<details>
<summary>Visualizar Captura de Tela (10 Clientes)</summary>

![Servidor - 10 Clientes](prints/Testes%20parte%201/Servidor_10_clientes.png)
![Cliente - 10 Clientes](prints/Testes%20parte%201/teste_10_clientes.png)
</details>

---

### Análise e Discussão dos Resultados

1. **Escalabilidade e Paralelismo Multithread:**
   - Com **1 cliente**, o throughput medido foi de **~20,65 Gbps**.
   - Conforme mais conexões concorrentes foram adicionadas, o paralelismo das threads do servidor e os múltiplos núcleos de processamento foram amplamente aproveitados, elevando a taxa para **~42,30 Gbps** com 2 clientes, **~56,97 Gbps** com 4 clientes e **~65,24 Gbps** com 5 clientes, culminando no pico máximo de **~72,10 Gbps** com 10 clientes.

2. **Comportamento em Loopback (127.0.0.1):**
   - No loopback, a comunicação não é limitada por uma placa física de rede (ex: 1 Gbps ou 10 Gbps), mas sim pela largura de banda da memória RAM, eficiência de cópia de buffers no kernel do Windows (Winsock) e capacidade de processamento da CPU. Por essa razão, taxas acima de dezenas de gigabits por segundo são observadas.

3. **Variações e Sobrecarga de Troca de Contexto (6, 8 e 9 Clientes):**
   - Nos testes com 6, 8 e 9 clientes, observou-se uma acomodação do throughput agregado na faixa de **~37 a 38 Gbps**. Esse fenômeno é comum em sistemas operacionais quando o número de threads concorrentes de alta taxa de I/O atinge o limite de núcleos físicos da máquina, gerando disputa de cache L2/L3 e custos de troca de contexto (*context switching*).

4. **Pico Máximo de Transferência (10 Clientes):**
   - No teste com 10 clientes, foi registrado o maior throughput agregado de todo o experimento: **72.099,59 Mbps (~72,10 Gbps)**, transferindo mais de **90,27 GB** em apenas 10 segundos, com distribuição consistente de dados entre todas as conexões ativas.

---

## Resultados dos Testes - Molinari (1 a 10 Clientes)

Os testes foram realizados em ambiente local (interface de *loopback* `127.0.0.1`), com duração configurada de **10 segundos** e blocos de **65.536 bytes (64 KiB)** por cliente.

### Ambiente de Teste

| Componente | Especificação |
|---|---|
| **Processador** | Intel Core i5-1245U (12ª geração) - 10 núcleos (2 P-cores + 8 E-cores), 12 threads, até 4,4 GHz |
| **Memória RAM** | 32 GB DDR5 (2 × 16 GB @ 4800 MT/s) |
| **Sistema Operacional** | Windows 11 |
| **Compilador** | GCC 15.2.0 (MSYS2 UCRT64), flag `-O3` |
| **Interface de Rede** | Loopback (`127.0.0.1`) |

### Como reproduzir

O script `rodar_testes.sh` compila os programas, executa automaticamente os testes de 1 a 10 clientes e exibe uma tabela-resumo ao final. Execute no **Git Bash** (ou terminal MSYS2), na pasta do projeto:

```bash
./rodar_testes.sh              # padrão: 1 a 10 clientes, 10 s, blocos de 65536 bytes
./rodar_testes.sh 10 10 65536  # equivalente, com parâmetros explícitos
```

### Tabela Comparativa de Desempenho

| Clientes Simultâneos | Duração Medida (s) | Total Recebido (Bytes) | Throughput Agregado (Mbps) | Throughput Agregado (Gbps) |
|:---:|:---:|:---:|:---:|:---:|
| **1** | 10.0006 s | 16.655.712.256 | 13.323,79 Mbps | ~13,32 Gbps |
| **2** | 10.0008 s | 34.328.805.376 | 27.460,92 Mbps | ~27,46 Gbps |
| **3** | 10.0008 s | 38.350.749.696 | 30.678,07 Mbps | ~30,68 Gbps |
| **4** | 10.0009 s | 49.876.828.160 | 39.897,73 Mbps | ~39,90 Gbps |
| **5** | 10.0012 s | 45.404.192.768 | 36.318,86 Mbps | ~36,32 Gbps |
| **6** | 10.0016 s | 40.281.571.328 | 32.219,96 Mbps | ~32,22 Gbps |
| **7** | 10.0012 s | 40.320.040.960 | 32.252,12 Mbps | ~32,25 Gbps |
| **8** | 10.0020 s | 41.004.892.160 | 32.797,49 Mbps | ~32,80 Gbps |
| **9** | 10.0024 s | 41.284.468.736 | 33.019,72 Mbps | ~33,02 Gbps |
| **10** | 10.0041 s | 40.450.129.920 | 32.346,72 Mbps | ~32,35 Gbps |

<details>
<summary>Visualizar Captura de Tela (bateria de 1 a 10 clientes)</summary>

![Bateria de testes - 1 a 10 clientes](prints/teste_completo_notebook.png)
</details>

---

### Detalhamento por Teste

#### 1 Cliente
* **Dados por cliente:**
  * Cliente 1: 16.655.712.256 bytes
* **Total recebido:** 16.655.712.256 bytes (~16,66 GB)
* **Tempo:** 10.0006 s
* **Throughput:** **13.323,79 Mbps** (~13,32 Gbps)

---

#### 2 Clientes
* **Dados por cliente:**
  * Cliente 1: 17.345.675.264 bytes
  * Cliente 2: 16.983.130.112 bytes
* **Total recebido:** 34.328.805.376 bytes (~34,33 GB)
* **Tempo:** 10.0008 s
* **Throughput:** **27.460,92 Mbps** (~27,46 Gbps)

---

#### 3 Clientes
* **Dados por cliente:**
  * Cliente 1: 12.817.530.880 bytes
  * Cliente 2: 12.677.021.696 bytes
  * Cliente 3: 12.856.197.120 bytes
* **Total recebido:** 38.350.749.696 bytes (~38,35 GB)
* **Tempo:** 10.0008 s
* **Throughput:** **30.678,07 Mbps** (~30,68 Gbps)

---

#### 4 Clientes
* **Dados por cliente:**
  * Cliente 1: 12.623.020.032 bytes
  * Cliente 2: 12.599.885.824 bytes
  * Cliente 3: 12.027.691.008 bytes
  * Cliente 4: 12.626.231.296 bytes
* **Total recebido:** 49.876.828.160 bytes (~49,88 GB)
* **Tempo:** 10.0009 s
* **Throughput:** **39.897,73 Mbps** (~39,90 Gbps)

---

#### 5 Clientes
* **Dados por cliente:**
  * Cliente 1: 9.138.667.520 bytes
  * Cliente 2: 9.023.717.376 bytes
  * Cliente 3: 9.114.746.880 bytes
  * Cliente 4: 9.154.920.448 bytes
  * Cliente 5: 8.972.140.544 bytes
* **Total recebido:** 45.404.192.768 bytes (~45,40 GB)
* **Tempo:** 10.0012 s
* **Throughput:** **36.318,86 Mbps** (~36,32 Gbps)

---

#### 6 Clientes
* **Dados por cliente:**
  * Cliente 1: 6.706.233.344 bytes
  * Cliente 2: 6.998.589.440 bytes
  * Cliente 3: 6.619.987.968 bytes
  * Cliente 4: 6.781.665.280 bytes
  * Cliente 5: 6.737.756.160 bytes
  * Cliente 6: 6.437.339.136 bytes
* **Total recebido:** 40.281.571.328 bytes (~40,28 GB)
* **Tempo:** 10.0016 s
* **Throughput:** **32.219,96 Mbps** (~32,22 Gbps)

---

#### 7 Clientes
* **Dados por cliente:**
  * Cliente 1: 5.658.836.992 bytes
  * Cliente 2: 5.696.126.976 bytes
  * Cliente 3: 5.814.943.744 bytes
  * Cliente 4: 5.665.980.416 bytes
  * Cliente 5: 5.843.451.904 bytes
  * Cliente 6: 5.904.007.168 bytes
  * Cliente 7: 5.736.693.760 bytes
* **Total recebido:** 40.320.040.960 bytes (~40,32 GB)
* **Tempo:** 10.0012 s
* **Throughput:** **32.252,12 Mbps** (~32,25 Gbps)

---

#### 8 Clientes
* **Dados por cliente:**
  * Cliente 1: 5.200.936.960 bytes
  * Cliente 2: 5.029.953.536 bytes
  * Cliente 3: 4.985.716.736 bytes
  * Cliente 4: 5.041.553.408 bytes
  * Cliente 5: 5.113.118.720 bytes
  * Cliente 6: 5.199.233.024 bytes
  * Cliente 7: 5.369.364.480 bytes
  * Cliente 8: 5.065.015.296 bytes
* **Total recebido:** 41.004.892.160 bytes (~41,00 GB)
* **Tempo:** 10.0020 s
* **Throughput:** **32.797,49 Mbps** (~32,80 Gbps)

---

#### 9 Clientes
* **Dados por cliente:**
  * Cliente 1: 4.271.833.088 bytes
  * Cliente 2: 4.587.782.144 bytes
  * Cliente 3: 4.737.335.296 bytes
  * Cliente 4: 4.628.938.752 bytes
  * Cliente 5: 4.753.195.008 bytes
  * Cliente 6: 4.468.047.872 bytes
  * Cliente 7: 4.546.822.144 bytes
  * Cliente 8: 4.588.961.792 bytes
  * Cliente 9: 4.701.552.640 bytes
* **Total recebido:** 41.284.468.736 bytes (~41,28 GB)
* **Tempo:** 10.0024 s
* **Throughput:** **33.019,72 Mbps** (~33,02 Gbps)

---

#### 10 Clientes
* **Dados por cliente:**
  * Cliente 1: 3.856.007.168 bytes
  * Cliente 2: 4.073.914.368 bytes
  * Cliente 3: 4.058.972.160 bytes
  * Cliente 4: 4.091.150.336 bytes
  * Cliente 5: 4.258.267.136 bytes
  * Cliente 6: 4.379.705.344 bytes
  * Cliente 7: 4.010.082.304 bytes
  * Cliente 8: 3.942.187.008 bytes
  * Cliente 9: 3.867.672.576 bytes
  * Cliente 10: 3.912.171.520 bytes
* **Total recebido:** 40.450.129.920 bytes (~40,45 GB)
* **Tempo:** 10.0041 s
* **Throughput:** **32.346,72 Mbps** (~32,35 Gbps)

---

### Análise dos Resultados

1. **Escalabilidade até 4 Clientes:** O throughput agregado cresceu de **~13,32 Gbps** com 1 cliente para **~39,90 Gbps** com 4 clientes, cerca de 3 vezes mais. De 1 para 2 clientes o throughput praticamente dobrou (~27,46 Gbps). Como os testes foram feitos via loopback, o envio em paralelo aproveita vários núcleos da CPU por meio do modelo multithread do servidor.
2. **Saturação a partir de 5 Clientes:** Depois do pico com 4 clientes, o throughput caiu para ~36,32 Gbps com 5 clientes e se estabilizou entre **~32,2 e ~33,0 Gbps** de 6 a 10 clientes. Cada cliente usa duas threads ocupadas (uma no processo cliente, que envia, e uma no servidor, que recebe). Com 6 clientes, isso já soma 12 threads ativas, o mesmo número de threads lógicas do i5-1245U, ou seja, a CPU fica totalmente ocupada. Acima disso, os clientes passam a dividir os mesmos núcleos, e o custo adicional de troca de contexto, de disputa por cache e memória e de uso dos E-cores, mais lentos, anula o ganho de adicionar mais clientes.
3. **Equilíbrio de Carga:** Nos testes com múltiplos clientes, os volumes transferidos individualmente ficaram muito próximos entre si: o cliente que mais enviou transmitiu no máximo ~14% a mais que o que menos enviou (e apenas 1–2% nos testes com 2, 3 e 5 clientes). Isso mostra uma divisão justa de recursos pelo escalonador do SO e pela pilha de rede.
4. **Dependência do Hardware:** Em loopback, os dados não passam por uma placa de rede; o throughput é limitado pela CPU e pela memória. Por isso, os valores absolutos variam bastante entre máquinas e até entre execuções na mesma máquina.

---

## Estrutura do repositório

```text
.
├── cliente.c                   # Cliente TCP para transmissão contínua de dados
├── servidor.c                  # Servidor TCP multithread de medição de throughput
├── Makefile                    # Script de compilação automatizada
├── prints/
│   └── Testes parte 1/         # Capturas de tela dos testes (1 a 10 clientes)
├── README.md                   # Documentação do projeto e resultados dos testes
└── .gitignore
```
