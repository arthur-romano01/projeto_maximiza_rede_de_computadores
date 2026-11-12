#!/usr/bin/env bash
# Bateria de testes de throughput, padrao: 10 clientes, 10 segundos, blocos de 65536 bytes.

MAX=${1:-10}
DURACAO=${2:-10}
BLOCO=${3:-65536}
PORTA_BASE=5100
LOGS="logs"

cd "$(dirname "$0")" || exit 1

# Localizar o GCC (MSYS2)
if ! command -v gcc >/dev/null 2>&1; then
    for d in /c/msys64/ucrt64/bin /c/msys64/mingw64/bin; do
        [ -x "$d/gcc.exe" ] && export PATH="$d:$PATH" && break
    done
fi
command -v gcc >/dev/null 2>&1 || { echo "GCC nao encontrado. Instale o MinGW/MSYS2."; exit 1; }

echo "Compilando..."
gcc -O3 servidor.c -o servidor.exe -lws2_32 || exit 1
gcc -O3 cliente.c  -o cliente.exe  -lws2_32 || exit 1

mkdir -p "$LOGS"
echo "Executando testes: 1 a $MAX clientes, ${DURACAO}s, blocos de $BLOCO bytes"
echo "(tempo estimado: ~$(( MAX * (DURACAO + 3) ))s)"
echo

for n in $(seq 1 "$MAX"); do
    porta=$((PORTA_BASE + n))
    printf "  Teste com %2d cliente(s)... " "$n"

    ./servidor.exe "$porta" "$n" > "$LOGS/servidor_$n.txt" 2>&1 &
    spid=$!
    sleep 1

    cpids=()
    for i in $(seq 1 "$n"); do
        ./cliente.exe 127.0.0.1 "$porta" "$DURACAO" "$BLOCO" > "$LOGS/cliente_${n}_$i.txt" 2>&1 &
        cpids+=($!)
    done
    wait "${cpids[@]}"
    wait "$spid"

    if grep -qi "erro" "$LOGS/servidor_$n.txt" "$LOGS"/cliente_${n}_*.txt; then
        echo "ERRO (ver $LOGS/)"
    else
        echo "ok"
    fi
    sleep 2
done

# Tabela-resumo final
echo
echo "==================================== RESULTADOS ===================================="
printf "| %-8s | %-12s | %-22s | %-15s | %-9s |\n" "Clientes" "Tempo (s)" "Total recebido (bytes)" "Throughput Mbps" "Gbps"
echo "|----------|--------------|------------------------|-----------------|-----------|"
for n in $(seq 1 "$MAX"); do
    f="$LOGS/servidor_$n.txt"
    total=$(sed -n 's/^Total recebido: \([0-9]*\) bytes.*/\1/p' "$f" | tr -d '\r')
    tempo=$(sed -n 's/^Tempo medido: \([0-9.]*\).*/\1/p' "$f" | tr -d '\r')
    mbps=$(sed -n 's/^Throughput agregado: \([0-9.]*\).*/\1/p' "$f" | tr -d '\r')
    gbps=$(awk -v m="$mbps" 'BEGIN { printf "%.2f", m / 1000 }')
    printf "| %8s | %12s | %22s | %15s | %9s |\n" "$n" "$tempo" "$total" "$mbps" "$gbps"
done
echo "===================================================================================="
echo "Duracao configurada: ${DURACAO}s | Bloco: $BLOCO bytes | Logs completos em ./$LOGS/"
