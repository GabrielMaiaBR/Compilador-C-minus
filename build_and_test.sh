#!/bin/bash

# Script para automatizar o processo de build e teste

# Saia imediatamente se um comando falhar
set -e

echo "Finalizando builds anteriores..."
./killbuild

echo "Entrando no diretório de build..."
cd build

echo "Executando CMake..."
cmake ..

echo "Construindo o alvo 'ddiff'..."
make ddiff

echo "Voltando para o diretório raiz..."
cd ..

echo "Executando o script de comparação de diffs..."
python compare_diffs.py

echo "Processo concluído com sucesso!"

