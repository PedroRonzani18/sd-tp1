# Trabalho Pratico 1 - Sistemas Distribuidos

Implementacoes em C++17 para os mecanismos de comunicacao e sincronizacao pedidos
no Trabalho Pratico 1. O projeto deve ser compilado e executado em Linux ou outro
sistema POSIX.

| Atividade | Diretorio | Tema |
| --- | --- | --- |
| 1 | `atividade-1-sinais` | Emissor e receptor de sinais, com busy wait e blocking wait |
| 2 | `atividade-2-pipes` | Produtor-consumidor com dois processos e pipe anonimo |
| 3 | `atividade-3-semaforos` | Produtor-consumidor multithreaded com buffer compartilhado e semaforos |

Cada atividade possui um `README.md` com comando de compilacao, execucao e roteiro
de testes. A terceira tambem inclui os scripts de benchmark e de geracao dos
graficos exigidos pelo enunciado.

## Dependencias

- `g++` com suporte a C++17;
- `make`;
- para os benchmarks: `bash` e `python3`.
