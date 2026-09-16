CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic
SRC_DIR := src
BIN_DIR := bin

.PHONY: all clean

all: $(BIN_DIR)/emissor.out $(BIN_DIR)/receptor.out

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/emissor.out: $(SRC_DIR)/emissor.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR)/receptor.out: $(SRC_DIR)/receptor.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(BIN_DIR)/emissor.out $(BIN_DIR)/receptor.out
