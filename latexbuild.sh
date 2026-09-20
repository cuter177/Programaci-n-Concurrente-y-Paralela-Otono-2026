#!/usr/bin/env bash
# ============================================
# Uso:
#   latexbuild main.tex
# ============================================

# Verificar argumento
if [ $# -eq 0 ]; then
    echo "Uso: latexbuild archivo.tex"
    exit 1
fi

MAIN="$1"

# Verificar existencia
if [ ! -f "$MAIN" ]; then
    echo "Error: no existe '$MAIN'"
    exit 1
fi

# Nombre base
BASENAME=$(basename "${MAIN%.tex}")

# Limpiar build anterior
rm -rf build

# Crear carpeta build
mkdir -p build

# Compilar
latexmk \
    -xelatex \
    -interaction=nonstopmode \
    -file-line-error \
    -output-directory=build \
    "$MAIN"

# Verificar resultado
if [ $? -eq 0 ]; then
    echo ""
    echo "Compilación exitosa"
    echo "PDF: build/$BASENAME.pdf"

    # Abrir automáticamente
    # Descomenta si quieres auto-open:
    # zathura "build/$BASENAME.pdf" &
else
    echo ""
    echo "Error durante la compilación"
    exit 1
fi

