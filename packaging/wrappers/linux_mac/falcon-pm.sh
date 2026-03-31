#!/bin/bash
NATS_ENV="${NATS_URL:-nats://host.docker.internal:4222}"

docker run -it --rm \
  -v "$(pwd):/workspace" \
  -w /workspace \
  -e NATS_URL="$NATS_ENV" \
  falcon-cli:latest falcon-pm "$@"
