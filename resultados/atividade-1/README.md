# Resultados - Atividade 1

Os testes foram executados em ambiente Linux após compilação com `g++` e as opções `-std=c++17 -Wall -Wextra -pedantic`.

| Caso | Resultado |
| --- | --- |
| Blocking wait + programa emissor | Aprovado. `SIGUSR1` e `SIGUSR2` foram tratados e `SIGTERM` encerrou o receptor. |
| Busy wait + programa emissor | Aprovado. Os três sinais foram tratados corretamente. |
| Blocking wait + comando `kill` | Aprovado. O receptor reagiu aos sinais enviados diretamente pelo sistema. |
| PID inexistente | Aprovado. O emissor informou o erro e retornou código de saída 1. |
| Comparação aproximada de CPU | Blocking: 0,0%; busy: 100% na amostra realizada. |

A comparação de CPU é apenas demonstrativa. Ela evidencia a diferença entre o processo bloqueado, que aguarda a chegada de um sinal sem ocupar continuamente o processador, e o busy wait, que permanece executando o laço de espera.

A saída bruta da execução está em `testes.txt`.
