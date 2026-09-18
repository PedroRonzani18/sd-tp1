#include <array>
#include <cerrno>
#include <charconv>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <string_view>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {

constexpr std::size_t MESSAGE_SIZE = 20;
using Message = std::array<char, MESSAGE_SIZE>;

bool write_all(int file_descriptor, const char* data, std::size_t size) {
    std::size_t total_written = 0;

    while (total_written < size) {
        const ssize_t written =
            write(file_descriptor, data + total_written, size - total_written);

        if (written > 0) {
            total_written += static_cast<std::size_t>(written);
        } else if (written == -1 && errno == EINTR) {
            continue;
        } else {
            return false;
        }
    }

    return true;
}

enum class ReadResult { success, end_of_file, error };

ReadResult read_message(int file_descriptor, Message& message) {
    std::size_t total_read = 0;

    while (total_read < message.size()) {
        const ssize_t bytes_read =
            read(file_descriptor, message.data() + total_read, message.size() - total_read);

        if (bytes_read > 0) {
            total_read += static_cast<std::size_t>(bytes_read);
        } else if (bytes_read == 0) {
            return ReadResult::end_of_file;
        } else if (errno == EINTR) {
            continue;
        } else {
            return ReadResult::error;
        }
    }

    return ReadResult::success;
}

bool encode_number(std::uint64_t number, Message& message) {
    message.fill('\0');
    const auto result = std::to_chars(message.data(), message.data() + message.size(), number);
    return result.ec == std::errc{};
}

bool decode_number(const Message& message, std::uint64_t& number) {
    const char* const begin = message.data();
    const char* end = begin;

    while (end != begin + message.size() && *end != '\0') {
        ++end;
    }

    if (begin == end) {
        return false;
    }

    const auto result = std::from_chars(begin, end, number);
    return result.ec == std::errc{} && result.ptr == end;
}

bool is_prime(std::uint64_t number) {
    if (number < 2) {
        return false;
    }
    if (number == 2) {
        return true;
    }
    if (number % 2 == 0) {
        return false;
    }

    for (std::uint64_t divisor = 3; divisor <= number / divisor; divisor += 2) {
        if (number % divisor == 0) {
            return false;
        }
    }

    return true;
}

bool parse_quantity(std::string_view text, std::uint64_t& quantity) {
    if (text.empty()) {
        return false;
    }

    const auto result = std::from_chars(text.data(), text.data() + text.size(), quantity);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

int run_producer(int write_end, std::uint64_t quantity) {
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<std::uint64_t> increment_distribution(1, 100);

    std::uint64_t current_number = 1;

    for (std::uint64_t index = 0; index < quantity; ++index) {
        const std::uint64_t increment = increment_distribution(generator);

        if (current_number > std::numeric_limits<std::uint64_t>::max() - increment) {
            std::cerr << "Produtor: o numero excedeu o limite de representacao.\n";
            return EXIT_FAILURE;
        }

        current_number += increment;

        Message message{};
        if (!encode_number(current_number, message) ||
            !write_all(write_end, message.data(), message.size())) {
            std::cerr << "Produtor: erro ao escrever no pipe: " << std::strerror(errno)
                      << '\n';
            return EXIT_FAILURE;
        }
    }

    Message termination_message{};
    if (!encode_number(0, termination_message) ||
        !write_all(write_end, termination_message.data(), termination_message.size())) {
        std::cerr << "Produtor: erro ao enviar a mensagem de termino: "
                  << std::strerror(errno) << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int run_consumer(int read_end) {
    while (true) {
        Message message{};
        const ReadResult read_result = read_message(read_end, message);

        if (read_result == ReadResult::error) {
            std::cerr << "Consumidor: erro ao ler o pipe: " << std::strerror(errno) << '\n';
            return EXIT_FAILURE;
        }
        if (read_result == ReadResult::end_of_file) {
            std::cerr << "Consumidor: o pipe foi fechado antes da mensagem de termino.\n";
            return EXIT_FAILURE;
        }

        std::uint64_t number = 0;
        if (!decode_number(message, number)) {
            std::cerr << "Consumidor: foi recebida uma mensagem invalida.\n";
            return EXIT_FAILURE;
        }

        if (number == 0) {
            break;
        }

        std::cout << "Consumidor: " << number
                  << (is_prime(number) ? " e primo.\n" : " nao e primo.\n");
    }

    return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <quantidade_de_numeros>\n";
        return EXIT_FAILURE;
    }

    std::uint64_t quantity = 0;
    if (!parse_quantity(argv[1], quantity)) {
        std::cerr << "Erro: a quantidade deve ser um numero inteiro nao negativo.\n";
        return EXIT_FAILURE;
    }

    // Cria o pipe antes do fork para que pai e filho compartilhem suas pontas.
    int pipe_ends[2];
    if (pipe(pipe_ends) == -1) {
        std::cerr << "Erro ao criar o pipe: " << std::strerror(errno) << '\n';
        return EXIT_FAILURE;
    }

    const pid_t child_pid = fork();
    if (child_pid == -1) {
        std::cerr << "Erro ao criar o processo consumidor: " << std::strerror(errno) << '\n';
        close(pipe_ends[0]);
        close(pipe_ends[1]);
        return EXIT_FAILURE;
    }


    // Cada processo fecha a ponta do pipe que nao usa.
    if (child_pid == 0) {
        close(pipe_ends[1]);
        const int consumer_status = run_consumer(pipe_ends[0]);
        close(pipe_ends[0]);
        return consumer_status;
    }

    close(pipe_ends[0]);
    const int producer_status = run_producer(pipe_ends[1], quantity);
    close(pipe_ends[1]);

    int child_status = 0;
    while (waitpid(child_pid, &child_status, 0) == -1) {
        if (errno != EINTR) {
            std::cerr << "Produtor: erro ao aguardar o consumidor: "
                      << std::strerror(errno) << '\n';
            return EXIT_FAILURE;
        }
    }

    const bool consumer_succeeded =
        WIFEXITED(child_status) && WEXITSTATUS(child_status) == EXIT_SUCCESS;

    return producer_status == EXIT_SUCCESS && consumer_succeeded ? EXIT_SUCCESS
                                                                  : EXIT_FAILURE;
}
