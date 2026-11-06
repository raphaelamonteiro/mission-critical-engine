# Bloco 1 — Variáveis
CC        := gcc
CFLAGS    := -std=c11 -Wall -Wextra -Wpedantic -g -O2
BUILD_DIR := build
SRC       := src/main.c
TARGET    := $(BUILD_DIR)/engine.exe

# Bloco 2 — .PHONY
.PHONY: all run clean

# Bloco 3 — Alvo all
all: $(TARGET)

# Bloco 4 — Regra do $(TARGET)
$(TARGET): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Bloco 5 — Regra da $(BUILD_DIR)
$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

# Bloco 6 — run
run: $(TARGET)
	.\$(TARGET)

# Bloco 7 — clean
clean:
	-rmdir /s /q $(BUILD_DIR)

