#!/bin/bash
set -e

# Falcon DSL Smoke Test Script
# This script runs a minimal autotuner to ensure the language environment is correctly set up.

echo "=========================================="
echo "Running Falcon DSL Smoke Test"
echo "=========================================="

AUTOTUNER_NAME="SimpleBool"
AUTOTUNER_FILE="/src/dsl/tests/test-autotuners/simple_bool/simple_bool.fal"

if [ ! -f "$AUTOTUNER_FILE" ]; then
    echo "Error: Autotuner file not found: $AUTOTUNER_FILE"
    exit 1
fi

echo "Debug Info:"
echo "PATH: $PATH"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
echo "Listing /opt/falcon/bin:"
ls -l /opt/falcon/bin || echo "/opt/falcon/bin not found"
echo "Listing /opt/falcon/lib:"
ls -l /opt/falcon/lib || echo "/opt/falcon/lib not found"
echo "Checking for falcon-run:"
which falcon-run || echo "falcon-run not in PATH"
echo "File Info:"
file $(which falcon-run) || echo "file command failed"
echo "LDD Info:"
ldd $(which falcon-run) || echo "ldd command failed"

echo "Running autotuner: $AUTOTUNER_NAME from $AUTOTUNER_FILE"

# Run falcon-run and capture output
# We expect it to complete with success and print the result
# We wrap it in a subshell to avoid set -e killing the script if it returns 127
output=$(falcon-run "$AUTOTUNER_NAME" "$AUTOTUNER_FILE" 2>&1) || {
    err=$?
    echo "Execution failed with exit code $err"
    echo "Output: $output"
    exit $err
}

echo "------------------------------------------"
echo "$output"
echo "------------------------------------------"

if echo "$output" | grep -q "Autotuner 'SimpleBool' completed" && echo "$output" | grep -q "\[0\] true"; then
    echo "✓ Smoke test PASSED"
    echo "=========================================="
    exit 0
else
    echo "✗ Smoke test FAILED"
    echo "=========================================="
    exit 1
fi
