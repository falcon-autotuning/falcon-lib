#!/bin/bash
docker run -it --rm \
  -v "$(pwd):/workspace" \
  -v falcon-config:/config:ro \
  -w /workspace \
  -e NATS_URL="${NATS_URL:-nats://host.docker.internal:4222}" \
  falcon-cli:latest falcon-run "$@"
