#!/bin/bash
set -e

# Falcon DSL Smoke Test Script
# This script runs a minimal autotuner to ensure the language environment is correctly set up.

echo "=========================================="
echo "Running Falcon DSL Smoke Test"
echo "=========================================="

# Check if falcon-run executes without shared library errors (LD_LIBRARY_PATH test)
output=$(falcon-run --help 2>&1 || true)

echo "------------------------------------------"
echo "$output"
echo "------------------------------------------"

if echo "$output" | grep -q "Usage: falcon-run"; then
    echo "✓ Smoke test PASSED (falcon-run is accessible)"
    echo "=========================================="
    exit 0
else
    echo "✗ Smoke test FAILED"
    echo "=========================================="
    exit 1
fi
