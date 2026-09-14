#include <csignal>
#include <cstring>
#include <iostream>
#include <unistd.h>

volatile sig_atomic_t executando = 1;

// write() e utilizado dentro dos handlers por ser apropriado para este contexto.
void tratar_sigusr1(int) {
    const char mensagem[] = "Recebido SIGUSR1.\n";
    write(STDOUT_FILENO, mensagem, sizeof(mensagem) - 1);
}

void tratar_sigusr2(int) {
    const char mensagem[] = "Recebido SIGUSR2.\n";
    write(STDOUT_FILENO, mensagem, sizeof(mensagem) - 1);
}

void tratar_sigterm(int) {
    const char mensagem[] = "Recebido SIGTERM. Encerrando.\n";
    write(STDOUT_FILENO, mensagem, sizeof(mensagem) - 1);
    executando = 0;
}

void registrar_handler(int sinal, void (*handler)(int)) {
    struct sigaction acao {};
    acao.sa_handler = handler;
    sigemptyset(&acao.sa_mask);
    acao.sa_flags = 0;
    sigaction(sinal, &acao, nullptr);
}

int main(int argc, char* argv[]) {
    if (argc != 2 ||
        (std::strcmp(argv[1], "busy") != 0 && std::strcmp(argv[1], "blocking") != 0)) {
        std::cerr << "Uso: " << argv[0] << " <busy|blocking>\n";
        return 1;
    }

    registrar_handler(SIGUSR1, tratar_sigusr1);
    registrar_handler(SIGUSR2, tratar_sigusr2);
    registrar_handler(SIGTERM, tratar_sigterm);

    std::cout << "PID: " << getpid() << " | modo: " << argv[1] << std::endl;

    if (std::strcmp(argv[1], "busy") == 0) {
        // Busy wait: o processo permanece executando enquanto aguarda um sinal.
        while (executando) {
        }
    } else {
        // Blocking wait: pause() bloqueia o processo ate a chegada de um sinal.
        while (executando) {
            pause();
        }
    }

    return 0;
}
