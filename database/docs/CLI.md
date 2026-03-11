# falcon-db-cli Reference

`falcon-db-cli` is the command-line interface for the Falcon Database module. It allows you to initialize the database schema and perform snapshot management (exporting, importing, and validating) without writing C++ code.

---

## Synopsis

```bash
falcon-db-cli [options] <command> [args...]
```

---

## Options

| Option | Argument | Description |
|--------|----------|-------------|
| `--db` | `url` | Override the `FALCON_DATABASE_URL` environment variable to connect to a specific database. |

---

## Commands

| Command | Arguments | Description |
|---------|-----------|-------------|
| `init` | — | Initialize the database schema. Safe to run multiple times (idempotent). |
| `schema` | — | Print the JSON schema for the snapshot format. |
| `snapshot export` | `<file>` | Export all device characteristics from the database to a JSON file. |
| `snapshot import` | `<file> [--clear]` | Import device characteristics from a JSON file into the database. If `--clear` is specified, it deletes all existing characteristics first. |
| `snapshot validate` | `<file>` | Validate a JSON file against the embedded schema without connecting to the database. |
| `help` | — | Show the help message and exit. |

---

## Exit codes

| Code | Meaning |
|------|---------|
| `0` | Command ran to completion |
| `1` | Argument / usage error, or a runtime error occurred (e.g. database connection failed, invalid JSON file) |

---

## Examples

### Initialize the database schema

```bash
falcon-db-cli init
```

### Export a snapshot

```bash
falcon-db-cli snapshot export snapshot_2026.json
```

### Import a snapshot (and clear existing)

```bash
falcon-db-cli snapshot import snapshot_2026.json --clear
```

### Use a custom database URL

```bash
falcon-db-cli --db "postgresql://user:pass@localhost/mydb" snapshot export my_data.json
```

### Validate a snapshot file

Validating does not require a database connection.

```bash
falcon-db-cli snapshot validate data.json
```

---

## Environment variables

| Variable | Effect |
|----------|--------|
| `FALCON_DATABASE_URL` | The PostgreSQL connection string to use. Can be overridden by the `--db` option. |
