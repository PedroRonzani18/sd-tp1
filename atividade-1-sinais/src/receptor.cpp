#include <cerrno>
#include <csignal>
#include <cstring>
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

bool registrar_handler(int sinal, void (*handler)(int)) {
    struct sigaction acao {};
    sigemptyset(&acao.sa_mask);
    acao.sa_flags = 0;
    acao.sa_handler = handler;

    return sigaction(sinal, &acao, nullptr) == 0;
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

    if (!registrar_handler(SIGUSR1, tratar_sigusr1) ||
        !registrar_handler(SIGUSR2, tratar_sigusr2) ||
        !registrar_handler(SIGTERM, tratar_sigterm)) {
        cerr << "Erro ao registrar os handlers: " << strerror(errno) << '\n';
        return 1;
    }

    cout << "PID: " << getpid() << " | modo: " << modo << endl;

    if (modo == "busy") {
        // O programa fica usando CPU continuamente enquanto espera.
        while (executando) {
            mostrar_sinais_recebidos();
        }
    } else {
        // Bloqueia os sinais antes de testar a condicao. Assim, um sinal nao se
        // perde no intervalo entre testar "executando" e iniciar a espera.
        sigset_t sinais_monitorados {};
        sigemptyset(&sinais_monitorados);
        sigaddset(&sinais_monitorados, SIGUSR1);
        sigaddset(&sinais_monitorados, SIGUSR2);
        sigaddset(&sinais_monitorados, SIGTERM);

        sigset_t mascara_anterior {};
        if (sigprocmask(SIG_BLOCK, &sinais_monitorados, &mascara_anterior) == -1) {
            cerr << "Erro ao bloquear sinais: " << strerror(errno) << '\n';
            return 1;
        }

        // sigsuspend() troca a mascara e espera de forma atomica, eliminando a
        // condicao de corrida que existiria com while (...) { pause(); }.
        sigset_t mascara_espera = mascara_anterior;
        sigdelset(&mascara_espera, SIGUSR1);
        sigdelset(&mascara_espera, SIGUSR2);
        sigdelset(&mascara_espera, SIGTERM);

        while (executando) {
            mostrar_sinais_recebidos();
            if (!executando) {
                break;
            }

            sigsuspend(&mascara_espera);
            mostrar_sinais_recebidos();
        }

        if (sigprocmask(SIG_SETMASK, &mascara_anterior, nullptr) == -1) {
            cerr << "Erro ao restaurar mascara de sinais: " << strerror(errno) << '\n';
            return 1;
        }
    }

    // Mostra a mensagem do SIGTERM antes de finalizar.
    mostrar_sinais_recebidos();
    return 0;
}
