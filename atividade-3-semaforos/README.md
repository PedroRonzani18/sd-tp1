# Atividade 3 - Produtor-Consumidor com semaforos

O programa implementa um buffer circular compartilhado por multiplas threads
produtoras e consumidoras. O buffer e protegido por tres semaforos POSIX:

- `mutex`: serializa o acesso aos indices, contadores e ao vetor;
- `empty_slots`: conta as posicoes livres e bloqueia produtores quando o buffer esta cheio;
- `filled_slots`: conta as posicoes ocupadas e bloqueia consumidores quando o buffer esta vazio.

Cada produtor gera inteiros aleatorios no intervalo `[1, 10^7]`. Cada consumidor
remove um item e informa se ele e primo. A execucao produz e consome exatamente
`M` itens; por padrao, `M = 100000`.

## Compilacao

```bash
cd atividade-3-semaforos
make
```

E necessario usar Linux ou outro ambiente POSIX com `g++`, `pthread` e semaforos
POSIX. Para gerar os graficos, o script tambem requer `python3`; nao ha bibliotecas
Python adicionais.

## Execucao manual

```bash
./bin/producer_consumer_semaphores.out <N> <Np> <Nc> [M=100000] [arquivo_ocupacao.csv]
```

Exemplo pequeno, adequado para inspecao manual:

```bash
./bin/producer_consumer_semaphores.out 10 2 2 30 ocupacao.csv
```

O arquivo opcional recebe uma linha para cada producao e consumo, com a ocupacao
do buffer imediatamente apos a operacao. O tempo e os contadores finais sao
impressos em `stderr`, para que a saida das classificacoes possa ser redirecionada
durante os benchmarks.

## Experimento exigido

```bash
bash ./scripts/benchmark.sh
```

O script executa 280 medicoes: `N = 1, 10, 100, 1000`, as sete combinacoes de
threads do enunciado e dez repeticoes por combinacao. A primeira repeticao de cada
cenario tambem registra o historico de ocupacao representativo. Ao final, os
arquivos gerados sao:

- `results/timings.csv`: todas as medicoes individuais;
- `results/occupancy/`: 28 historicos de ocupacao;
- `results/figures/tempo_medio.svg`: medias por configuracao, com uma curva para cada `N`;
- `results/figures/ocupacao_*.svg`: um grafico de ocupacao para cada cenario.

Para um teste rapido sem alterar o codigo, reduza a quantidade total:

```bash
TOTAL_ITEMS=1000 bash ./scripts/benchmark.sh
```
