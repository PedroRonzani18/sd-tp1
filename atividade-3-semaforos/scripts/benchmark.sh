#!/usr/bin/env bash
set -euo pipefail

# Executa exatamente os cenarios pedidos pelo enunciado: quatro capacidades,
# sete combinacoes de threads e dez repeticoes por combinacao.
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BINARY="$ROOT_DIR/bin/producer_consumer_semaphores.out"
RESULTS_DIR="$ROOT_DIR/results"
TOTAL_ITEMS="${TOTAL_ITEMS:-100000}"
REPETITIONS=10

capacities=(1 10 100 1000)
scenarios=("1 1" "1 2" "1 4" "1 8" "2 1" "4 1" "8 1")

make -C "$ROOT_DIR"
mkdir -p "$RESULTS_DIR/occupancy" "$RESULTS_DIR/figures"
printf 'N,produtores,consumidores,execucao,tempo_ms\n' > "$RESULTS_DIR/timings.csv"

for capacity in "${capacities[@]}"; do
    for scenario in "${scenarios[@]}"; do
        read -r producers consumers <<< "$scenario"

        for repetition in $(seq 1 "$REPETITIONS"); do
            occupancy_file=""
            if [[ "$repetition" -eq 1 ]]; then
                occupancy_file="$RESULTS_DIR/occupancy/N${capacity}_P${producers}_C${consumers}.csv"
            fi

            if [[ -n "$occupancy_file" ]]; then
                program_output=$("$BINARY" "$capacity" "$producers" "$consumers" \
                    "$TOTAL_ITEMS" "$occupancy_file" 2>&1 > /dev/null)
            else
                program_output=$("$BINARY" "$capacity" "$producers" "$consumers" \
                    "$TOTAL_ITEMS" 2>&1 > /dev/null)
            fi

            elapsed_ms=$(awk '/^Tempo de execucao:/ { print $4 }' <<< "$program_output")
            if [[ -z "$elapsed_ms" ]]; then
                echo "Nao foi possivel obter o tempo para N=$capacity P=$producers C=$consumers." >&2
                echo "$program_output" >&2
                exit 1
            fi

            printf '%s,%s,%s,%s,%s\n' "$capacity" "$producers" "$consumers" \
                "$repetition" "$elapsed_ms" >> "$RESULTS_DIR/timings.csv"
        done
    done
done

python3 "$ROOT_DIR/scripts/plot_results.py" "$RESULTS_DIR/timings.csv" \
    "$RESULTS_DIR/occupancy" "$RESULTS_DIR/figures"

echo "Resultados gravados em $RESULTS_DIR"
