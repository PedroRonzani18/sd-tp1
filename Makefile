CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic
BIN_DIR := bin
ATV1_DIR := atividade-1-sinais

.PHONY: all atividade1 test-atividade1 clean

all: atividade1

atividade1: $(BIN_DIR)/emissor $(BIN_DIR)/receptor

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/emissor: $(ATV1_DIR)/emissor.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR)/receptor: $(ATV1_DIR)/receptor.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

test-atividade1: atividade1
	./atividade-1-sinais/testar.sh

clean:
	rm -rf $(BIN_DIR)
