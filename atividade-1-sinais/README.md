# Atividade 1 — Sinais

## Roteiro para testar

Entre na pasta da atividade e compile os programas:

```bash
cd atividade-1-sinais
make
```

Abra dois terminais nessa pasta.

### Terminal 1: receptor

Inicie o receptor no modo bloqueante:

```bash
./bin/receptor.out blocking
```

Ele exibirá algo parecido com:

```text
PID: 12345 | modo: blocking
```

Anote o PID exibido.

### Terminal 2: emissor

Copie o PID mostrado no Terminal 1 para uma variável do shell. Exemplo, se o
PID exibido foi `12345`:

```bash
RECEPTOR_PID=12345
```

Depois envie os sinais usando a variável:

```bash
./bin/emissor.out "$RECEPTOR_PID" "$(kill -l USR1)"
./bin/emissor.out "$RECEPTOR_PID" "$(kill -l USR2)"
./bin/emissor.out "$RECEPTOR_PID" "$(kill -l TERM)"
```

Ao iniciar outro receptor, basta atualizar a variável com o novo PID:

```bash
RECEPTOR_PID=NOVO_PID
```

O receptor deve mostrar mensagens para `SIGUSR1` e `SIGUSR2`. Ao receber
`SIGTERM`, ele mostra a mensagem final e encerra.

## Teste usando `kill`

Com o receptor aberto novamente, também é possível enviar os sinais sem usar
o programa emissor:

```bash
kill -USR1 "$RECEPTOR_PID"
kill -USR2 "$RECEPTOR_PID"
kill -TERM "$RECEPTOR_PID"
```

## Teste do busy wait

No Terminal 1, troque `blocking` por `busy`:

```bash
./bin/receptor.out busy
```

Depois repita os mesmos comandos do Terminal 2.

## Comparação de uso de CPU

Use dois terminais. No Terminal 1, inicie o receptor em modo `busy`:

```bash
./bin/receptor.out busy
```

No Terminal 2, copie o PID exibido e espere um segundo. Em seguida, veja o
uso de CPU do processo:

```bash
RECEPTOR_PID=12345
sleep 1
ps -p "$RECEPTOR_PID" -o pid=,%cpu=,stat=,command=
```

O modo `busy` deve usar uma porcentagem alta de CPU. Encerre-o:

```bash
kill -TERM "$RECEPTOR_PID"
```

Agora, no Terminal 1, inicie o modo `blocking`:

```bash
./bin/receptor.out blocking
```

No Terminal 2, atualize a variável com o novo PID e execute o mesmo comando:

```bash
RECEPTOR_PID=NOVO_PID
sleep 1
ps -p "$RECEPTOR_PID" -o pid=,%cpu=,stat=,command=
```

No modo `blocking`, o uso de CPU deve ser próximo de `0.0` e o estado tende a
aparecer como `S` (sleeping). No modo `busy`, o estado tende a aparecer como
`R` (running).
