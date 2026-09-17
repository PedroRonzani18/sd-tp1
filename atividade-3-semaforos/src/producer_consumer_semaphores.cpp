#include <charconv>
#include <chrono>
#include <cstdint>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <mutex>
#include <random>
#include <semaphore.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

constexpr std::uint64_t DEFAULT_TOTAL_ITEMS = 100000;
constexpr std::uint32_t MIN_RANDOM_VALUE = 1;
constexpr std::uint32_t MAX_RANDOM_VALUE = 10000000;

struct Configuration {
    std::size_t capacity;
    std::size_t producer_count;
    std::size_t consumer_count;
    std::uint64_t total_items;
    std::string occupancy_file;
};

class SharedBuffer {
public:
    explicit SharedBuffer(std::size_t capacity, bool record_occupancy)
        : values_(capacity), record_occupancy_(record_occupancy) {
        if (sem_init(&mutex_, 0, 1) == -1) {
            throw std::runtime_error(std::strerror(errno));
        }
        mutex_initialized_ = true;

        if (sem_init(&empty_slots_, 0, static_cast<unsigned int>(capacity)) == -1) {
            sem_destroy(&mutex_);
            throw std::runtime_error(std::strerror(errno));
        }
        empty_slots_initialized_ = true;

        if (sem_init(&filled_slots_, 0, 0) == -1) {
            sem_destroy(&empty_slots_);
            sem_destroy(&mutex_);
            throw std::runtime_error(std::strerror(errno));
        }
        filled_slots_initialized_ = true;

        if (record_occupancy_) {
            occupancy_history_.reserve(capacity * 2U);
        }
    }

    SharedBuffer(const SharedBuffer&) = delete;
    SharedBuffer& operator=(const SharedBuffer&) = delete;

    ~SharedBuffer() {
        if (filled_slots_initialized_) {
            sem_destroy(&filled_slots_);
        }
        if (empty_slots_initialized_) {
            sem_destroy(&empty_slots_);
        }
        if (mutex_initialized_) {
            sem_destroy(&mutex_);
        }
    }

    sem_t& mutex() { return mutex_; }
    sem_t& empty_slots() { return empty_slots_; }
    sem_t& filled_slots() { return filled_slots_; }

    std::vector<std::uint32_t>& values() { return values_; }
    std::size_t& write_index() { return write_index_; }
    std::size_t& read_index() { return read_index_; }
    std::size_t& occupied() { return occupied_; }
    std::uint64_t& produced() { return produced_; }
    std::uint64_t& consumed() { return consumed_; }
    bool& production_finished() { return production_finished_; }
    std::vector<std::size_t>& occupancy_history() { return occupancy_history_; }
    bool records_occupancy() const { return record_occupancy_; }
    std::mutex& output_mutex() { return output_mutex_; }

private:
    std::vector<std::uint32_t> values_;
    std::size_t write_index_ = 0;
    std::size_t read_index_ = 0;
    std::size_t occupied_ = 0;
    std::uint64_t produced_ = 0;
    std::uint64_t consumed_ = 0;
    bool production_finished_ = false;
    bool record_occupancy_ = false;
    std::vector<std::size_t> occupancy_history_;
    std::mutex output_mutex_;
    sem_t mutex_ {};
    sem_t empty_slots_ {};
    sem_t filled_slots_ {};
    bool mutex_initialized_ = false;
    bool empty_slots_initialized_ = false;
    bool filled_slots_initialized_ = false;
};

void wait_semaphore(sem_t& semaphore) {
    while (sem_wait(&semaphore) == -1) {
        if (errno != EINTR) {
            throw std::runtime_error(std::strerror(errno));
        }
    }
}

void post_semaphore(sem_t& semaphore) {
    if (sem_post(&semaphore) == -1) {
        throw std::runtime_error(std::strerror(errno));
    }
}

bool is_prime(std::uint32_t number) {
    if (number < 2U) {
        return false;
    }
    if (number == 2U) {
        return true;
    }
    if (number % 2U == 0U) {
        return false;
    }

    for (std::uint32_t divisor = 3U; divisor <= number / divisor; divisor += 2U) {
        if (number % divisor == 0U) {
            return false;
        }
    }
    return true;
}

void producer(SharedBuffer& buffer, const Configuration& configuration, std::size_t producer_id) {
    std::random_device device;
    std::seed_seq seed {device(), device(), static_cast<unsigned int>(producer_id)};
    std::mt19937 generator(seed);
    std::uniform_int_distribution<std::uint32_t> distribution(MIN_RANDOM_VALUE,
                                                               MAX_RANDOM_VALUE);

    while (true) {
        wait_semaphore(buffer.empty_slots());
        wait_semaphore(buffer.mutex());

        if (buffer.produced() == configuration.total_items) {
            post_semaphore(buffer.mutex());
            post_semaphore(buffer.empty_slots());
            return;
        }

        buffer.values()[buffer.write_index()] = distribution(generator);
        buffer.write_index() = (buffer.write_index() + 1U) % buffer.values().size();
        ++buffer.occupied();
        ++buffer.produced();

        if (buffer.records_occupancy()) {
            buffer.occupancy_history().push_back(buffer.occupied());
        }

        post_semaphore(buffer.mutex());
        post_semaphore(buffer.filled_slots());
    }
}

void consumer(SharedBuffer& buffer, std::size_t consumer_id) {
    while (true) {
        wait_semaphore(buffer.filled_slots());
        wait_semaphore(buffer.mutex());

        // Depois que todos os produtores terminam, a thread principal libera
        // cada consumidor bloqueado com um post adicional em filled_slots.
        if (buffer.production_finished() && buffer.occupied() == 0U) {
            post_semaphore(buffer.mutex());
            return;
        }

        const std::uint32_t number = buffer.values()[buffer.read_index()];
        buffer.read_index() = (buffer.read_index() + 1U) % buffer.values().size();
        --buffer.occupied();
        ++buffer.consumed();

        if (buffer.records_occupancy()) {
            buffer.occupancy_history().push_back(buffer.occupied());
        }

        post_semaphore(buffer.mutex());
        post_semaphore(buffer.empty_slots());

        const bool prime = is_prime(number);
        std::lock_guard<std::mutex> output_lock(buffer.output_mutex());
        std::cout << "Consumidor " << consumer_id << ": " << number
                  << (prime ? " e primo.\n" : " nao e primo.\n");
    }
}

bool parse_unsigned(std::string_view text, std::uint64_t& result) {
    if (text.empty()) {
        return false;
    }

    const auto conversion = std::from_chars(text.data(), text.data() + text.size(), result);
    return conversion.ec == std::errc{} && conversion.ptr == text.data() + text.size();
}

bool parse_size(std::string_view text, std::size_t& result) {
    std::uint64_t value = 0;
    if (!parse_unsigned(text, value) || value == 0U ||
        value > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        return false;
    }

    result = static_cast<std::size_t>(value);
    return true;
}

bool parse_configuration(int argc, char* argv[], Configuration& configuration) {
    if (argc < 4 || argc > 6 || !parse_size(argv[1], configuration.capacity) ||
        !parse_size(argv[2], configuration.producer_count) ||
        !parse_size(argv[3], configuration.consumer_count) ||
        configuration.capacity > std::numeric_limits<unsigned int>::max()) {
        return false;
    }

    configuration.total_items = DEFAULT_TOTAL_ITEMS;
    if (argc >= 5 && !parse_unsigned(argv[4], configuration.total_items)) {
        return false;
    }

    if (argc == 6) {
        configuration.occupancy_file = argv[5];
        if (configuration.occupancy_file.empty()) {
            return false;
        }
    }

    return true;
}

bool write_occupancy_file(const std::string& path, const std::vector<std::size_t>& history) {
    std::ofstream output(path);
    if (!output) {
        return false;
    }

    output << "operacao,ocupacao\n";
    for (std::size_t index = 0; index < history.size(); ++index) {
        output << (index + 1U) << ',' << history[index] << '\n';
    }

    return static_cast<bool>(output);
}

}  // namespace

int main(int argc, char* argv[]) {
    Configuration configuration {};
    if (!parse_configuration(argc, argv, configuration)) {
        std::cerr << "Uso: " << argv[0]
                  << " <N> <numero_produtores> <numero_consumidores>"
                  << " [M=100000] [arquivo_ocupacao.csv]\n";
        return 1;
    }

    try {
        SharedBuffer buffer(configuration.capacity, !configuration.occupancy_file.empty());
        const auto start = std::chrono::steady_clock::now();

        std::vector<std::thread> producers;
        producers.reserve(configuration.producer_count);
        for (std::size_t index = 0; index < configuration.producer_count; ++index) {
            producers.emplace_back(producer, std::ref(buffer), std::cref(configuration),
                                   index + 1U);
        }

        std::vector<std::thread> consumers;
        consumers.reserve(configuration.consumer_count);
        for (std::size_t index = 0; index < configuration.consumer_count; ++index) {
            consumers.emplace_back(consumer, std::ref(buffer), index + 1U);
        }

        for (std::thread& thread : producers) {
            thread.join();
        }

        wait_semaphore(buffer.mutex());
        buffer.production_finished() = true;
        post_semaphore(buffer.mutex());

        for (std::size_t index = 0; index < configuration.consumer_count; ++index) {
            post_semaphore(buffer.filled_slots());
        }

        for (std::thread& thread : consumers) {
            thread.join();
        }

        const auto end = std::chrono::steady_clock::now();
        const std::chrono::duration<double, std::milli> elapsed = end - start;

        if (buffer.produced() != configuration.total_items ||
            buffer.consumed() != configuration.total_items || buffer.occupied() != 0U) {
            std::cerr << "Erro: a execucao terminou com contadores inconsistentes.\n";
            return 1;
        }

        if (!configuration.occupancy_file.empty() &&
            !write_occupancy_file(configuration.occupancy_file, buffer.occupancy_history())) {
            std::cerr << "Erro ao gravar o historico de ocupacao em "
                      << configuration.occupancy_file << ".\n";
            return 1;
        }

        std::cerr << "Tempo de execucao: " << elapsed.count() << " ms\n";
        std::cerr << "Itens produzidos/consumidos: " << buffer.produced() << '/' << buffer.consumed()
                  << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Erro de sincronizacao: " << error.what() << '\n';
        return 1;
    }
}
