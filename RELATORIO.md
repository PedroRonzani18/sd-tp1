# Trabalho Pratico 1 - Sistemas Distribuidos

**Aluno:** Pedro Augusto de Portilho Ronzani  
**Disciplina:** Sistemas Distribuidos - CEFET-MG  
**Professora:** Michelle Hanne

## 1. Objetivo

O trabalho explora sinais entre processos, comunicacao por pipe anonimo e o problema produtor-consumidor com threads e semaforos. As implementacoes usam C++17 em ambiente POSIX. Codigo-fonte: `https://github.com/PedroRonzani18/sd-tp1`.

## 2. Sinais

O emissor recebe PID e numero de sinal, verifica a existencia/permissao do processo com `kill(pid, 0)` e envia o sinal solicitado. O receptor instala handlers para `SIGUSR1`, `SIGUSR2` e `SIGTERM`; os dois primeiros produzem mensagens distintas e o ultimo encerra o programa.

O receptor aceita `busy` ou `blocking`. No primeiro modo, ha espera ocupada. No segundo, os sinais monitorados sao bloqueados antes da verificacao da condicao e `sigsuspend()` realiza a espera com troca atomica de mascara, evitando a corrida entre testar a condicao e esperar um sinal. Os testes incluem envio pelo emissor, pelo comando `kill`, PID inexistente e comparacao de CPU com `ps`.

## 3. Pipes

O programa produtor-consumidor cria um pipe antes do `fork()`. O pai fecha a ponta de leitura e produz a sequencia `Ni = N(i-1) + delta`, com `N0 = 1` e incremento aleatorio entre 1 e 100. O filho fecha a ponta de escrita, le cada mensagem e classifica o valor como primo ou nao primo.

Cada numero e serializado em uma mensagem fixa de 20 bytes. As rotinas de leitura e escrita repetem a chamada de sistema ate transferir todos os bytes, inclusive em caso de interrupcao por sinal. Depois da quantidade solicitada, o produtor envia zero como sentinela; o pai fecha o pipe e usa `waitpid()` para recolher o consumidor. Foram previstos testes de entrada invalida, producao vazia, dez numeros e a carga de mil numeros.

## 4. Semaforos

A terceira atividade usa um vetor circular compartilhado de capacidade `N`, com `Np` threads produtoras e `Nc` consumidoras. Tres semaforos POSIX controlam o acesso: `mutex` protege o vetor, indices e contadores; `empty_slots` bloqueia produtores com buffer cheio; e `filled_slots` bloqueia consumidores com buffer vazio. Cada produtor gera valores aleatorios entre 1 e `10^7`, e cada consumidor verifica a primalidade do item removido.

O programa reserva e processa exatamente `M = 100000` itens por padrao. Depois de todas as produtoras encerrarem, a thread principal sinaliza as consumidoras bloqueadas. A estrategia nao insere sentinelas no vetor e preserva a contagem exata de itens. Opcionalmente, o programa gera um CSV com a ocupacao do buffer apos cada producao ou consumo.

## 5. Estudos de caso e avaliacao

Os roteiros de teste das Atividades 1 e 2 estao nos respectivos READMEs. Para a Atividade 3, `bash atividade-3-semaforos/scripts/benchmark.sh` executa `N = 1, 10, 100, 1000`, as sete combinacoes de `Np/Nc` exigidas e dez repeticoes de cada combinacao: 280 medicoes. O script grava `timings.csv`, gera `tempo_medio.svg` e cria 28 graficos representativos de ocupacao em `results/figures`.

**Preenchimento obrigatorio antes da entrega:** executar o benchmark em Linux, inserir o grafico de tempo medio neste relatorio e registrar a analise observada. Nao devem ser usados valores simulados. A analise deve comparar o efeito da capacidade do buffer, do desbalanceamento produtor/consumidor e da contencao no semaforo de exclusao mutua. Os CSVs e os graficos de ocupacao permanecem no repositorio como evidencia dos 28 cenarios.

## 6. Conclusao

As tres implementacoes exercitam mecanismos distintos do sistema operacional: sinais para notificacao assincrona, pipes para fluxo entre processos e semaforos para coordenacao de threads e memoria compartilhada. O repositorio inclui compilacao, testes e automacao para reproduzir os resultados.
