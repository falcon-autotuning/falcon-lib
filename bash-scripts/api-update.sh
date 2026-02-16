#!/bin/bash

# Bash script to update the API version in the specified files
# run this in the src directory to update to the latest API version
export GOPRIVATE=*/falcon-autotuning
go install github.com/falcon-autotuning/falcon-api/cmd/cpp-api-loader@dev

cpp-api-loader --repo=falcon --output=./comms/include/falcon-comms/api.cpp
