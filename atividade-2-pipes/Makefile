CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -O2
SRC_DIR := src
BIN_DIR := bin

.PHONY: all clean

all: $(BIN_DIR)/producer_consumer.out

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/producer_consumer.out: $(SRC_DIR)/producer_consumer.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(BIN_DIR)/producer_consumer.out
