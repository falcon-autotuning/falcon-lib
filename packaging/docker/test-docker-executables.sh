#!/bin/bash
set -e

# Test script for Falcon Docker container executables
# Usage: ./test-docker-executables.sh <image_name>

IMAGE_NAME="${1:-falcon-cli:latest}"

echo "=========================================="
echo "Testing Falcon Docker image: $IMAGE_NAME"
echo "=========================================="

test_executable() {
    local cmd=$1
    local expected_regex=$2
    local args=$3

    echo -n "Testing $cmd... "
    output=$(docker run --rm "$IMAGE_NAME" "$cmd" $args 2>&1)
    
    if [[ $output =~ $expected_regex ]]; then
        echo "PASSED"
    else
        echo "FAILED"
        echo "Expected output to match: $expected_regex"
        echo "Actual output:"
        echo "------------------------------------------"
        echo "$output"
        echo "------------------------------------------"
        exit 1
    fi
}

# 1. Test falcon-run
test_executable "falcon-run" "Usage: falcon-run <autotuner-name> <file.fal>" "--help"

# 2. Test falcon-test
test_executable "falcon-test" "Usage: falcon-test <file.fal>" "--help"

# 3. Test falcon-pm
test_executable "falcon-pm" "falcon-pm: A package manager for the Falcon DSL" "--help"

# 4. Test falcon-db-cli
test_executable "falcon-db-cli" "falcon-db-cli: A command-line interface for the Falcon Database" "help"

echo "=========================================="
echo "✓ All executable tests PASSED"
echo "=========================================="
