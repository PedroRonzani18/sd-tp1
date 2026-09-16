#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>

using namespace std;

bool converter_inteiro_positivo(const string& texto, int& resultado) {
    try {
        size_t caracteres_processados = 0;
        resultado = stoi(texto, &caracteres_processados);

        return caracteres_processados == texto.size() && resultado > 0;
    } catch (...) {
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <pid> <sinal>\n";
        return 1;
    }

    int valor_pid = 0;
    if (!converter_inteiro_positivo(argv[1], valor_pid)) {
        cerr << "Erro: PID invalido.\n";
        return 1;
    }

    int valor_sinal = 0;
    if (!converter_inteiro_positivo(argv[2], valor_sinal)) {
        cerr << "Erro: sinal invalido.\n";
        return 1;
    }

    const pid_t pid = valor_pid;
    const int numero_sinal = valor_sinal;

    // O sinal 0 nao e entregue. Ele apenas verifica se o processo existe
    // e se o usuario possui permissao para sinaliza-lo.
    if (kill(pid, 0) == -1 && errno != EPERM) {
        cerr << "Erro: processo " << pid << " nao encontrado.\n";
        return 1;
    }

    if (kill(pid, numero_sinal) == -1) {
        cerr << "Erro ao enviar sinal: " << strerror(errno) << "\n";
        return 1;
    }

    cout << "Sinal " << numero_sinal << " enviado ao processo " << pid << ".\n";
    return 0;
}
