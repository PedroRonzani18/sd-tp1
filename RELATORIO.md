# Trabalho Pratico 1 - Sistemas Distribuidos

**Aluno:** Pedro Augusto de Portilho Ronzani  
**Disciplina:** Sistemas Distribuidos - CEFET-MG  
**Professora:** Michelle Hanne

## 1. Objetivo

O trabalho explora sinais entre processos, comunicacao por pipe anonimo e o problema produtor-consumidor com threads e semaforos. As implementacoes usam C++17 em ambiente POSIX. Codigo-fonte: `https://github.com/PedroRonzani18/sd-tp1`.

## 2. Sinais

O emissor recebe PID e numero de sinal, verifica a existencia/permissao do processo com `kill(pid, 0)` e envia o sinal solicitado. O receptor instala handlers para `SIGUSR1`, `SIGUSR2` e `SIGTERM`; os dois primeiros produzem mensagens distintas e o ultimo encerra o programa.

O receptor aceita `busy` ou `blocking`. No primeiro modo, ha espera ocupada, com uso continuo da CPU. No segundo, `pause()` suspende o processo ate a chegada de um sinal. Esta implementacao simples pode sofrer uma corrida se um sinal chegar entre a verificacao de `executando` e a chamada de `pause()`. Os testes propostos incluem envio pelo emissor e pelo comando `kill`, PID inexistente e comparacao de CPU com `ps`.

## 3. Pipes: dois processos e um canal de dados

Nesta versao do problema produtor-consumidor, o processo pai produz numeros e o filho os recebe e testa sua primalidade. O programa chama `pipe()` antes de `fork()` para que ambos herdem as pontas do mesmo canal. Em seguida, o pai fecha a ponta de leitura e o filho fecha a ponta de escrita. Assim, a comunicacao ocorre em uma direcao: pai -> pipe -> filho.

O produtor parte de `N0 = 1` e gera a sequencia crescente `Ni = N(i-1) + delta`, com incremento aleatorio entre 1 e 100. O pipe transporta bytes, nao numeros ou mensagens delimitadas automaticamente. Por isso, cada valor e convertido para uma mensagem fixa de 20 bytes; as rotinas de escrita e leitura repetem as chamadas ate transferir a mensagem inteira, tratando inclusive interrupcoes por sinal. O consumidor decodifica cada valor recebido e imprime se ele e primo.

A quantidade de numeros e um argumento do programa. Apos o ultimo valor, o pai envia `0` como sentinela; o filho reconhece o fim sem classificar esse zero. O pai fecha a escrita e usa `waitpid()` para aguardar o termino do filho. Na demonstracao, uma execucao com dez valores permite observar a sequencia crescente e as classificacoes; os valores exatos variam devido ao sorteio. O README tambem descreve casos com entrada invalida, producao vazia e mil numeros.

## 4. Semaforos: varias threads e um buffer compartilhado

Nesta versao, `Np` threads produtoras e `Nc` consumidoras compartilham um vetor circular de capacidade `N`. Os indices retornam ao inicio do vetor quando chegam a ultima posicao, permitindo reutilizar espacos liberados. Cada produtora gera inteiros aleatorios entre 1 e `10^7`; cada consumidora retira um item e verifica se ele e primo. Como varias threads podem acessar os mesmos indices e contadores, o acesso sem coordenacao causaria condicoes de corrida.

Tres semaforos POSIX resolvem problemas diferentes. `mutex` comeca em 1 e permite que apenas uma thread altere o vetor, os indices e os contadores por vez. `empty_slots` comeca em `N` e faz a produtora esperar quando nao ha vaga. `filled_slots` comeca em 0 e faz a consumidora esperar quando nao ha item. A produtora espera uma vaga, entra na regiao critica, insere o valor, sai e anuncia um item. A consumidora espera um item, entra na regiao critica, retira o valor, sai e anuncia uma vaga. O teste de primalidade ocorre fora da regiao critica.

O programa produz e consome exatamente `M` itens, sendo `M = 100000` por padrao. Depois que todas as produtoras terminam, a thread principal acorda as consumidoras que ainda estiverem bloqueadas, sem colocar sentinelas no vetor. Para a demonstracao, `N = 10`, `Np = 2`, `Nc = 2` e `M = 30` permitem conferir o total produzido/consumido sem gerar uma saida longa. Um CSV opcional registra a ocupacao do buffer apos cada producao ou consumo.

## 5. Estudos de caso e avaliacao

### 5.1. Testes funcionais e criterios de verificacao

Os comandos abaixo devem ser executados em Linux ou outro ambiente POSIX, com `g++` e `make`, a partir da pasta de cada atividade. Os READMEs contêm o passo a passo completo. Nesta revisao do relatorio, o projeto nao foi compilado nem executado novamente; portanto, os criterios esperados nao devem ser confundidos com resultados medidos.

| Atividade e caso | Comando ou procedimento | Criterio de verificacao |
| --- | --- | --- |
| Sinais: envio valido | Iniciar `./bin/receptor.out blocking` e enviar `SIGUSR1`, `SIGUSR2` e `SIGTERM` pelo emissor e por `kill`. | Mensagens distintas para os tres sinais; o receptor encerra apos `SIGTERM`. |
| Sinais: erro e espera | Informar PID inexistente ao emissor; comparar `busy` e `blocking` com `ps`. | Erro para PID inexistente; `busy` tende a consumir CPU, enquanto `blocking` tende a esperar sem uso continuo de CPU. |
| Pipes: entradas limite | Executar `./bin/producer_consumer.out -1` e `./bin/producer_consumer.out 0`. | Entrada negativa rejeitada; zero itens nao produz classificacoes. |
| Pipes: demonstracao | Executar `./bin/producer_consumer.out 10`. | Dez classificacoes; numeros crescentes, com incrementos entre 1 e 100; o zero sentinela nao aparece como item classificado. |
| Pipes: carga maior | Executar `./bin/producer_consumer.out 1000` com a saida redirecionada e conferir `wc -l`. | Mil linhas de classificacao, sem truncamento ou duplicacao de mensagens. |
| Semaforos: demonstracao | Executar `./bin/producer_consumer_semaphores.out 10 2 2 30 ocupacao.csv`. | Contadores finais `30/30`; ocupacao registrada a cada operacao, sempre entre 0 e 10 e terminando em 0. |

O arquivo local `atividade-3-semaforos/ocupacao.csv`, correspondente a um exemplo com `N = 10` e `M = 30`, foi inspecionado separadamente: ele contem 60 registros de operacao, ocupacao minima 0, maxima 10 e valor final 0. Isso documenta o historico disponivel, mas nao substitui a reexecucao do programa nem comprova por si so o resultado dos 28 cenarios do experimento.

### 5.2. Metodo do experimento de desempenho

O script `bash atividade-3-semaforos/scripts/benchmark.sh` foi preparado para `M = 100000`, capacidades `N = 1, 10, 100, 1000`, combinacoes `(Np,Nc) = (1,1), (1,2), (1,4), (1,8), (2,1), (4,1), (8,1)` e dez repeticoes por configuracao. Sao `4 x 7 x 10 = 280` execucoes. `TOTAL_ITEMS=1000 bash ./scripts/benchmark.sh` reduz apenas `M` para uma verificacao rapida; nao representa a carga oficial de 100000 itens.

O programa mede, com `steady_clock`, o intervalo desde antes da criacao das threads ate depois de todas terminarem. O tempo inclui a criacao, a sincronizacao, o processamento e a impressao das classificacoes (redirecionadas para `/dev/null` pelo script). A gravacao final do CSV de ocupacao ocorre depois desse intervalo. Na primeira repeticao de cada configuracao, o historico de ocupacao e registrado em memoria durante a medicao; nas outras nove, nao. Essa diferenca, assim como a carga do computador, pode influenciar as medias e deve ser considerada ao compara-las.

O script grava as 280 medidas em `results/timings.csv`, calcula a media das dez repeticoes de cada configuracao e gera `results/figures/tempo_medio.svg`. O eixo horizontal representa as sete combinacoes de threads, o vertical representa o tempo medio em milissegundos e cada curva representa um valor de `N`. Menor tempo significa execucao mais rapida, mas mais threads ou um buffer maior nao garantem melhoria: CPU, impressao e contencao pela regiao critica tambem podem limitar o desempenho.

### 5.3. Graficos de ocupacao e limites da analise

A primeira repeticao de cada um dos 28 cenarios gera um CSV e um grafico `results/figures/ocupacao_*.svg`. O eixo horizontal e a ordem das operacoes de producao ou consumo, nao o tempo em segundos; o vertical e o numero de posicoes ocupadas, limitado ao intervalo de 0 a `N`. Subidas correspondem a producoes e descidas a consumos. Ocupacao frequentemente proxima de `N` e compativel com produtoras aguardando vagas; proxima de zero, com consumidoras aguardando itens. Esses graficos mostram a dinamica do buffer; a comparacao de velocidade depende das medidas de tempo.

**Pendente para a entrega final:** executar e conservar os 280 resultados reais em ambiente POSIX, conferir os totais produzidos/consumidos, inserir o grafico de tempo medio e analisar os valores observados. A analise deve comparar capacidade do buffer, equilibrio entre produtoras e consumidoras e possiveis fontes de contencao, registrando o ambiente de execucao. Nao foram incluidos tempos nem conclusoes numericas sem as medicoes correspondentes.

## 6. Conclusao

As tres implementacoes exercitam mecanismos distintos do sistema operacional: sinais para notificacao assincrona, pipes para fluxo entre processos e semaforos para coordenacao de threads e memoria compartilhada. O repositorio inclui compilacao, testes e automacao para reproduzir os resultados.
