#!/bin/bash
set -e

# (Optional) You can add runtime checks here.
# For example, checking if NATS_URL is reachable before starting.

# Hand off execution to the command passed into the container
# This will evaluate to things like: exec falcon-run --args
exec "$@"
