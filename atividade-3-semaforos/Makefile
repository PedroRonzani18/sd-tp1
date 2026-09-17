CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -O2 -pthread
SRC_DIR := src
BIN_DIR := bin

.PHONY: all clean

all: $(BIN_DIR)/producer_consumer_semaphores.out

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/producer_consumer_semaphores.out: $(SRC_DIR)/producer_consumer_semaphores.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(BIN_DIR)/producer_consumer_semaphores.out
