#!/usr/bin/env bash
# Installs Python dependencies for falcon qarrayDevice.
# Can be run standalone or called from the Makefile.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PYTHON="${PYTHON:-python3}"

echo "Installing Python dependencies for qarrayDevice..."
echo "Using Python: $("$PYTHON" --version)"

"$PYTHON" -m pip install --upgrade pip
"$PYTHON" -m pip install -r "$SCRIPT_DIR/requirements.txt"

echo ""
echo "✓ Python dependencies installed successfully."
echo ""
echo "Installed packages:"
"$PYTHON" -c "import qarray; print(f'  qarray     {qarray.__version__}')" 2>/dev/null || echo "  qarray     (version check failed)"
"$PYTHON" -c "import numpy;  print(f'  numpy      {numpy.__version__}')" 2>/dev/null || echo "  numpy      (version check failed)"
"$PYTHON" -c "import yaml;   print(f'  pyyaml     (ok)')" 2>/dev/null || echo "  pyyaml     (version check failed)"
