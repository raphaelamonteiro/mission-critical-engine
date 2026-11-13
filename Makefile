# ==============================================================================
# Mission Critical Engine — Makefile
# Resilient C11 Space Station Telemetry Simulator
# ==============================================================================

CC        := gcc
CFLAGS    := -std=c11 -Wall -Wextra -Wpedantic -g -O2 -Iinclude
BUILD_DIR := build
SRC_DIR   := src

# Descoberta de fontes e objetos via pattern rules
SRC       := $(wildcard $(SRC_DIR)/*.c)
OBJ       := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))
TARGET    := $(BUILD_DIR)/engine.exe

.PHONY: all run clean help

all: $(TARGET)

# Ligação (Linkagem)
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Pattern Rule: compilação incremental de cada unidade de tradução .c -> .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Criação do diretório de build como order-only prerequisite
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR) 2>/dev/null || mkdir $(BUILD_DIR) 2>nul || echo Directoy exists

run: $(TARGET)
	./$(TARGET)

clean:
	@rm -rf $(BUILD_DIR) 2>/dev/null || rmdir /s /q $(BUILD_DIR) 2>nul || echo Cleaned
