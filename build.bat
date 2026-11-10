@echo off
echo ========================================
echo COMPILANDO COM O GCC NOVO (MSYS2)...
echo ========================================
if not exist build mkdir build

:: Usando o caminho absoluto do GCC 16.2.0
C:\msys64\ucrt64\bin\gcc.exe -std=c11 -Wall -Wextra -Wpedantic -g -O2 -Iinclude src/main.c src/integrity.c -o build/engine.exe

if %ERRORLEVEL% EQU 0 (
    echo [SUCESSO] Compilado!
) else (
    echo [ERRO] Falha na compilacao.
)