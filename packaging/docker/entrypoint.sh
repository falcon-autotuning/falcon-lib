#!/bin/bash
set -e

# If the secure config volume is mounted, load the database URL
if [ -f /config/db.env ]; then
  source /config/db.env
  export DB_URL="$FALCON_DB_URL"
fi

# Execute the user's command
exec "$@"
