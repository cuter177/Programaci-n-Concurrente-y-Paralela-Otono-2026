#!/bin/bash
# Genera presentacion.pdf a partir de presentacion.html (reveal.js, modo ?print-pdf)
# Uso: ./generarpdf.sh
set -euo pipefail

cd "$(dirname "$0")"

HTML="presentacion.html"
PDF="presentacion.pdf"
TOOLS="$PWD/.tools"

[ -f "$HTML" ] || { echo "ERROR: no existe $HTML" >&2; exit 1; }

# 1) Dependencia (solo la primera vez)
if [ ! -d "$TOOLS/node_modules/puppeteer-core" ]; then
  echo "Instalando puppeteer-core en .tools/ (solo la primera vez)…"
  mkdir -p "$TOOLS"
  [ -f "$TOOLS/package.json" ] || printf '{\n  "name": "tools-pdf",\n  "private": true,\n  "version": "1.0.0"\n}\n' > "$TOOLS/package.json"
  (cd "$TOOLS" && npm install --silent puppeteer-core@23)
fi

# 2) Servidor local en un puerto libre (se cierra al terminar)
PORT=$(python3 - <<'EOF'
import socket
s = socket.socket()
s.bind(("127.0.0.1", 0))
print(s.getsockname()[1])
s.close()
EOF
)

python3 -m http.server "$PORT" --bind 127.0.0.1 >/dev/null 2>&1 &
SERVER_PID=$!
trap 'kill "$SERVER_PID" 2>/dev/null || true' EXIT INT TERM

# 3) Esperar a que responda (máx. 10 s)
for _ in $(seq 1 50); do
  curl -sSf -o /dev/null "http://127.0.0.1:$PORT/$HTML" 2>/dev/null && break
  sleep 0.2
done
curl -sSf -o /dev/null "http://127.0.0.1:$PORT/$HTML" 2>/dev/null \
  || { echo "ERROR: el servidor local no respondió." >&2; exit 1; }
echo "Servidor local listo (puerto $PORT)"

# 4) Renderizar el PDF (espera a que reveal.js termine antes de imprimir)
node "$TOOLS/topdf.js" "http://127.0.0.1:$PORT/$HTML?print-pdf" "$PWD/$PDF"

# 5) Verificación opcional del número de páginas
if command -v pdfinfo >/dev/null 2>&1; then
  N_SLIDES=$(grep -c "<section" "$HTML")
  N_PAGES=$(pdfinfo "$PDF" | awk '/^Pages:/{print $2}')
  echo "Diapositivas HTML: $N_SLIDES   Páginas PDF: $N_PAGES"
  [ "$N_PAGES" = "$N_SLIDES" ] || { echo "AVISO: no coinciden." >&2; exit 1; }
fi

echo "Listo: $PDF"
