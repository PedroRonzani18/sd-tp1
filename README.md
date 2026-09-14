# SDIF - Trabalho Prático 1

Trabalho Prático 1 da disciplina de **Sistemas Distribuídos** do CEFET-MG.

O repositório é organizado por atividade. Cada parte do trabalho fica em uma pasta própria, com a implementação e instruções de execução, enquanto os resultados dos testes ficam separados em `resultados/`. Dessa forma, as próximas atividades podem ser adicionadas seguindo o mesmo padrão sem misturar programas independentes.

## Estrutura atual

```text
sd-tp1/
├── atividade-1-sinais/
│   ├── emissor.cpp
│   ├── receptor.cpp
│   ├── testar.sh
│   └── README.md
├── resultados/
│   └── atividade-1/
│       ├── README.md
│       └── testes.txt
├── .gitignore
├── Makefile
└── README.md
```

## Requisitos

A implementação utiliza C++17 e interfaces POSIX. Os testes foram realizados em Linux.

É necessário ter disponível:

- `g++`;
- `make`;
- ambiente Linux/Unix com suporte a sinais POSIX.

## Atividade 1 - Sinais

A primeira atividade é composta por dois programas:

- **emissor:** recebe o PID do processo de destino e o número do sinal, verifica se o processo existe e envia o sinal;
- **receptor:** registra handlers para `SIGUSR1`, `SIGUSR2` e `SIGTERM` e aguarda sinais nos modos `busy` ou `blocking`.

O `SIGTERM` foi definido como o sinal responsável por encerrar o receptor.

### Compilar

Na raiz do repositório:

```bash
make atividade1
```

Os executáveis serão gerados em `bin/`:

```text
bin/emissor
bin/receptor
```

### Executar manualmente

Em um terminal, inicie o receptor:

```bash
./bin/receptor blocking
```

O programa mostrará o próprio PID. Em outro terminal, envie os sinais usando esse PID:

```bash
./bin/emissor <PID> 10   # SIGUSR1 no Linux
./bin/emissor <PID> 12   # SIGUSR2 no Linux
./bin/emissor <PID> 15   # SIGTERM no Linux
```

Também é possível executar o receptor com busy wait:

```bash
./bin/receptor busy
```

E os sinais podem ser enviados diretamente pelo comando `kill`:

```bash
kill -USR1 <PID>
kill -USR2 <PID>
kill -TERM <PID>
```

> Os números 10, 12 e 15 correspondem a `SIGUSR1`, `SIGUSR2` e `SIGTERM` no ambiente Linux utilizado nos testes. O script automatizado obtém esses números pelo próprio sistema.

## Testes automatizados

Para compilar e executar todos os testes da Atividade 1:

```bash
make test-atividade1
```

O script valida:

1. receptor em blocking wait com envio pelo programa emissor;
2. receptor em busy wait com envio pelo programa emissor;
3. envio dos sinais diretamente pelo comando `kill`;
4. tratamento de um PID inexistente;
5. uma comparação demonstrativa do uso de CPU entre blocking e busy wait.

A saída da execução fica registrada em:

```text
resultados/atividade-1/testes.txt
```

Uma síntese dos resultados está em `resultados/atividade-1/README.md`.

## Limpeza

Para remover os executáveis gerados:

```bash
make clean
```

## Organização das próximas atividades

As próximas partes do TP deverão seguir o mesmo padrão, criando uma pasta própria para cada atividade e uma pasta correspondente dentro de `resultados/`. O `Makefile` da raiz poderá receber novos alvos à medida que as implementações forem adicionadas.
