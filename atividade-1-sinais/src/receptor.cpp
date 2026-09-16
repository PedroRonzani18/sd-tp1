#include <csignal>
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

volatile sig_atomic_t executando = 1;
volatile sig_atomic_t recebeu_sigusr1 = 0;
volatile sig_atomic_t recebeu_sigusr2 = 0;
volatile sig_atomic_t recebeu_sigterm = 0;

// Cada funcao abaixo e chamada automaticamente pelo sistema operacional
// quando o processo recebe o sinal correspondente.
void tratar_sigusr1(int) {
    recebeu_sigusr1 = 1;
}

void tratar_sigusr2(int) {
    recebeu_sigusr2 = 1;
}

void tratar_sigterm(int) {
    recebeu_sigterm = 1;
    executando = 0;
}

// cout nao deve ser usado dentro dos handlers; por isso as mensagens sao
// impressas aqui, pelo fluxo normal do programa.
void mostrar_sinais_recebidos() {
    if (recebeu_sigusr1) {
        recebeu_sigusr1 = 0;
        cout << "Recebido SIGUSR1." << endl;
    }

    if (recebeu_sigusr2) {
        recebeu_sigusr2 = 0;
        cout << "Recebido SIGUSR2." << endl;
    }

    if (recebeu_sigterm) {
        recebeu_sigterm = 0;
        cout << "Recebido SIGTERM. Encerrando." << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <busy|blocking>\n";
        return 1;
    }

    const string modo = argv[1];
    if (modo != "busy" && modo != "blocking") {
        cerr << "Uso: " << argv[0] << " <busy|blocking>\n";
        return 1;
    }

    // "sigaction" e o tipo de configuracao que o sistema operacional entende.
    // "acao" e somente a variavel onde guardamos essa configuracao.
    struct sigaction acao {};
    // sa_mask seria uma lista de sinais a bloquear durante o tratamento.
    // A lista fica vazia: nao vamos bloquear nenhum sinal extra.
    sigemptyset(&acao.sa_mask);
    // sa_flags guarda opcoes extras. Zero significa usar o comportamento padrao,
    // sem ligar nenhuma opcao especial.
    acao.sa_flags = 0;

    acao.sa_handler = tratar_sigusr1; // Define qual funcao deve rodar ao receber SIGUSR1.

    // Entrega essa configuracao ao sistema. "&acao" e o endereco da variavel
    // de configuracao; "nullptr" diz que nao precisamos da configuracao antiga.
    sigaction(SIGUSR1, &acao, nullptr);

    acao.sa_handler = tratar_sigusr2;
    sigaction(SIGUSR2, &acao, nullptr);

    acao.sa_handler = tratar_sigterm;
    sigaction(SIGTERM, &acao, nullptr);

    cout << "PID: " << getpid() << " | modo: " << modo << endl;

    if (modo == "busy") {
        // O programa fica usando CPU continuamente enquanto espera.
        while (executando) {
            mostrar_sinais_recebidos();
        }
    } else {
        // pause() para o processo ate um sinal chegar.
        while (executando) {
            pause();
            mostrar_sinais_recebidos();
        }
    }

    // Mostra a mensagem do SIGTERM antes de finalizar.
    mostrar_sinais_recebidos();
    return 0;
}
