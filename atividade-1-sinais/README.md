# Atividade 1 - Sinais

Esta pasta contém os dois programas solicitados na atividade de sinais.

## Arquivos

- `emissor.cpp`: recebe um PID e o número de um sinal, verifica se o processo existe e envia o sinal;
- `receptor.cpp`: registra handlers para `SIGUSR1`, `SIGUSR2` e `SIGTERM` e aguarda sinais em modo `busy` ou `blocking`;
- `testar.sh`: executa os principais testes da atividade e registra a saída em `resultados/atividade-1/testes.txt`.

O `SIGTERM` foi escolhido como o sinal responsável por encerrar o receptor.

## Compilação

Execute na raiz do repositório:

```bash
make atividade1
```

## Execução

### Blocking wait

Terminal 1:

```bash
./bin/receptor blocking
```

O receptor exibirá seu PID. Use esse valor em um segundo terminal:

```bash
./bin/emissor <PID> 10
./bin/emissor <PID> 12
./bin/emissor <PID> 15
```

No ambiente Linux utilizado nos testes, esses números representam respectivamente `SIGUSR1`, `SIGUSR2` e `SIGTERM`.

Os mesmos sinais podem ser enviados sem o programa emissor:

```bash
kill -USR1 <PID>
kill -USR2 <PID>
kill -TERM <PID>
```

### Busy wait

Terminal 1:

```bash
./bin/receptor busy
```

O envio dos sinais é feito da mesma maneira.

## Comportamento esperado

Para `SIGUSR1` e `SIGUSR2`, o receptor imprime uma mensagem e continua executando. Ao receber `SIGTERM`, imprime a mensagem de encerramento e termina.

A diferença entre os modos está apenas na espera:

- `busy`: permanece executando um laço enquanto aguarda sinais;
- `blocking`: utiliza `pause()` e fica bloqueado enquanto não há sinal a tratar.

## Testes automatizados

Na raiz do repositório:

```bash
make test-atividade1
```

O resultado completo fica em `resultados/atividade-1/testes.txt`.
