# Atividade 3 - Produtor-Consumidor com semáforos

O programa usa um **buffer circular** (um vetor cujas posições são reutilizadas)
compartilhado por várias threads produtoras e consumidoras. As produtoras colocam
números no buffer; as consumidoras retiram esses números e verificam se são primos.
Três semáforos POSIX coordenam esse trabalho:

- `mutex`: permite que apenas uma thread altere o vetor, os índices e os contadores por vez;
- `empty_slots`: conta as posições livres e faz uma produtora esperar quando o buffer está cheio;
- `filled_slots`: conta os itens disponíveis e faz uma consumidora esperar quando o buffer está vazio.

Cada produtora gera inteiros aleatórios no intervalo `[1, 10^7]`. Ao todo, a
execução produz e consome exatamente `M` itens; por padrão, `M = 100000`.

## Compilação

```bash
cd atividade-3-semaforos
make
```

É necessário usar Linux ou outro ambiente POSIX com `g++`, `pthread` e semáforos
POSIX. Para gerar os gráficos, o script também requer `python3`; não há bibliotecas
Python adicionais.

## Execução manual

```bash
./bin/producer_consumer_semaphores.out <N> <Np> <Nc> [M=100000] [arquivo_ocupacao.csv]
```

Os argumentos aparecem **nesta ordem**:

| Argumento | Significado |
| --- | --- |
| `N` | Capacidade do buffer: quantos números cabem nele ao mesmo tempo. Por exemplo, `N = 10` significa dez posições. |
| `Np` | Número de **threads produtoras** criadas. Por exemplo, `Np = 2` cria duas produtoras. |
| `Nc` | Número de **threads consumidoras** criadas. Por exemplo, `Nc = 2` cria duas consumidoras. |
| `M` | Quantidade **total** de números produzidos e consumidos pela execução, não por thread. É opcional; se omitido, vale `100000`. |
| `arquivo_ocupacao.csv` | Arquivo opcional para registrar a ocupação do buffer após cada operação. Para informá-lo, também é preciso informar `M` antes. |

`N`, `Np` e `Nc` devem ser inteiros positivos. Eles representam coisas diferentes:
`N` mede **espaço no buffer**; `Np` e `Nc` medem **quantas threads** trabalham
em cada papel.

Exemplo pequeno, adequado para inspeção manual:

```bash
./bin/producer_consumer_semaphores.out 10 2 2 30 ocupacao.csv
```

Esse comando cria um buffer com **10 posições**, inicia **2 produtoras** e
**2 consumidoras**, e processa **30 números no total**. Ao terminar, o programa
deve informar `Itens produzidos/consumidos: 30/30`.

O arquivo opcional recebe uma linha por produção ou consumo, com a ocupação
do buffer imediatamente após a operação. O tempo e os contadores finais são
impressos em `stderr`, para que a saída das classificações possa ser redirecionada
durante os benchmarks.

## Experimento exigido

```bash
bash ./scripts/benchmark.sh
```

O script executa 280 medições: quatro capacidades de buffer (`N = 1, 10, 100,
1000`) × sete combinações de produtoras e consumidoras (`Np/Nc`) × dez repetições.
A primeira repetição de cada cenário também registra um histórico de ocupação.
Ao final, os arquivos gerados são:

- `results/timings.csv`: todas as medições individuais;
- `results/occupancy/`: 28 históricos de ocupação;
- `results/figures/tempo_medio.svg`: médias por configuração, com uma curva para cada `N`;
- `results/figures/ocupacao_*.svg`: um gráfico de ocupação para cada cenário.

Para um teste rápido sem alterar o código, reduza a quantidade total:

```bash
TOTAL_ITEMS=1000 bash ./scripts/benchmark.sh
```
