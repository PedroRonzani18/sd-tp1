# Atividade 2 - Produtor-Consumidor com pipes

Esta atividade usa um pipe anonimo e dois processos. O pai produz uma sequencia
crescente de inteiros e o filho consome cada valor, verificando se ele e primo.

## Compilacao

```bash
cd atividade-2-pipes
make
```

O programa requer Linux ou outro sistema POSIX, com `g++` disponivel.

## Execucao

```bash
./bin/producer_consumer.out <quantidade_de_numeros>
```

Exemplo:

```bash
./bin/producer_consumer.out 10
```

O produtor parte de `N0 = 1`, soma um incremento aleatorio de `1` a `100`
antes de cada envio e envia `0` ao final. Cada mensagem ocupa exatamente 20
bytes no pipe. O valor zero e a sentinela de termino e nao e classificado pelo
consumidor.

## Casos de teste

```bash
# Entrada invalida: deve encerrar com erro.
./bin/producer_consumer.out -1

# Producao vazia: nao deve haver classificacoes.
./bin/producer_consumer.out 0

# Inspecao manual da sequencia e das classificacoes.
./bin/producer_consumer.out 10

# Carga proposta no enunciado: devem ser produzidas 1000 linhas.
time ./bin/producer_consumer.out 1000 > /tmp/saida_pipes.txt
wc -l /tmp/saida_pipes.txt
head -n 5 /tmp/saida_pipes.txt
tail -n 5 /tmp/saida_pipes.txt
```
