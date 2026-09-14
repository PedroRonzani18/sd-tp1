#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"
make atividade1 >/dev/null

OUT="resultados/atividade-1/testes.txt"
mkdir -p "$(dirname "$OUT")"
: > "$OUT"

cleanup() {
    jobs -pr | xargs -r kill -TERM 2>/dev/null || true
}
trap cleanup EXIT

{
    echo "=== Teste 1: blocking wait + emissor ==="
    ./bin/receptor blocking > /tmp/sdif_receiver_blocking.log 2>&1 &
    pid=$!
    sleep 0.2
    ./bin/emissor "$pid" "$(kill -l USR1)"
    sleep 0.1
    ./bin/emissor "$pid" "$(kill -l USR2)"
    sleep 0.1
    ./bin/emissor "$pid" "$(kill -l TERM)"
    wait "$pid"
    cat /tmp/sdif_receiver_blocking.log
    echo

    echo "=== Teste 2: busy wait + emissor ==="
    ./bin/receptor busy > /tmp/sdif_receiver_busy.log 2>&1 &
    pid=$!
    sleep 0.2
    ./bin/emissor "$pid" "$(kill -l USR1)"
    sleep 0.1
    ./bin/emissor "$pid" "$(kill -l USR2)"
    sleep 0.1
    ./bin/emissor "$pid" "$(kill -l TERM)"
    wait "$pid"
    cat /tmp/sdif_receiver_busy.log
    echo

    echo "=== Teste 3: blocking wait + comando kill ==="
    ./bin/receptor blocking > /tmp/sdif_receiver_kill.log 2>&1 &
    pid=$!
    sleep 0.2
    kill -USR1 "$pid"
    sleep 0.1
    kill -USR2 "$pid"
    sleep 0.1
    kill -TERM "$pid"
    wait "$pid"
    cat /tmp/sdif_receiver_kill.log
    echo

    echo "=== Teste 4: processo inexistente ==="
    set +e
    ./bin/emissor 999999 "$(kill -l USR1)"
    status=$?
    set -e
    echo "codigo de saida: $status"
    echo

    echo "=== Teste 5: comparacao aproximada de CPU ==="
    ./bin/receptor blocking > /tmp/sdif_receiver_cpu_blocking.log 2>&1 &
    blocking_pid=$!
    ./bin/receptor busy > /tmp/sdif_receiver_cpu_busy.log 2>&1 &
    busy_pid=$!
    sleep 1
    ps -p "$blocking_pid" -o %cpu=,stat= | sed 's/^/blocking: /'
    ps -p "$busy_pid" -o %cpu=,stat= | sed 's/^/busy:     /'
    kill -TERM "$blocking_pid" "$busy_pid"
    wait "$blocking_pid" || true
    wait "$busy_pid" || true
} 2>&1 | tee "$OUT"
