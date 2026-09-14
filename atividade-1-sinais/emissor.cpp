#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <pid> <sinal>\n";
        return 1;
    }

    char* end = nullptr;
    long pid_value = std::strtol(argv[1], &end, 10);
    if (*argv[1] == '\0' || *end != '\0' || pid_value <= 0) {
        std::cerr << "Erro: PID invalido.\n";
        return 1;
    }

    end = nullptr;
    long signal_value = std::strtol(argv[2], &end, 10);
    if (*argv[2] == '\0' || *end != '\0' || signal_value <= 0) {
        std::cerr << "Erro: sinal invalido.\n";
        return 1;
    }

    pid_t pid = static_cast<pid_t>(pid_value);
    int signal_number = static_cast<int>(signal_value);

    // O sinal 0 nao e entregue. Ele apenas verifica se o processo existe
    // e se o usuario possui permissao para sinaliza-lo.
    if (kill(pid, 0) == -1 && errno != EPERM) {
        std::cerr << "Erro: processo " << pid << " nao encontrado.\n";
        return 1;
    }

    if (kill(pid, signal_number) == -1) {
        std::cerr << "Erro ao enviar sinal: " << std::strerror(errno) << "\n";
        return 1;
    }

    std::cout << "Sinal " << signal_number << " enviado ao processo " << pid << ".\n";
    return 0;
}
