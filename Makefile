# Bloco 1 — Variáveis
CC        := gcc
CFLAGS    := -std=c11 -Wall -Wextra -Wpedantic -g -O2 -Iinclude
BUILD_DIR := build
SRC       := $(wildcard src/*.c)
TARGET    := $(BUILD_DIR)/engine.exe

# Bloco 2 — .PHONY
.PHONY: all run clean

# Bloco 3 — Alvo all
all: $(TARGET)

# Bloco 4 — Compilação
$(TARGET): $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Bloco 5 — run
run: $(TARGET)
	./$(TARGET)

# Bloco 6 — clean
clean:
	rm -rf $(BUILD_DIR)