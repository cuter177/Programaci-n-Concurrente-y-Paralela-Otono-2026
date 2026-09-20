#!/usr/bin/env bash
# ============================================
# Instalador de dependencias para latexbuild.sh
# Distribución: Debian / Ubuntu
# Uso: sudo ./install-latexbuild.sh
# ============================================
set -euo pipefail

if [ "$(id -u)" -ne 0 ]; then
    echo "Error: ejecuta este script como root (sudo ./install-latexbuild.sh)"
    exit 1
fi

echo ">>> Actualizando índices de paquetes..."
apt-get update

echo ">>> Instalando dependencias de LaTeX y visor de PDF..."
apt-get install -y \
    latexmk \
    texlive-xetex \
    texlive-latex-base \
    texlive-latex-recommended \
    texlive-latex-extra \
    texlive-fonts-recommended \
    texlive-fonts-extra \
    texlive-lang-spanish \
    texlive-bibtex-extra \
    texlive-science \
    biber \
    zathura \
    ghostscript

echo ">>> Instalando latexbuild.sh en /usr/local/bin..."
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
install -m 0755 "$SCRIPT_DIR/latexbuild.sh" /usr/local/bin/latexbuild

echo ""
echo "Instalación completada."
echo "Prueba con: latexbuild archivo.tex"
