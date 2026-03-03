# falcon-run CLI Reference

`falcon-run` is the user-facing command-line interface for the Falcon AutotunerEngine. It allows you to load `.fal` files and execute any autotuner without writing C++ glue code.

---

## Synopsis

```bash
falcon-run <autotuner-name> <file.fal> [more.fal ...] [options]
falcon-run --list <file.fal> [more.fal ...]
```

---

## Options

| Option | Argument | Description |
|--------|----------|-------------|
| `--list` | — | List all autotuners discovered in the loaded files, then exit |
| `--param` | `k=v` | Set an input parameter; type is inferred (bool → int → float → string). Repeatable. |
| `--log-level` | `L` | Set verbosity: `trace`, `debug`, `info` (default), `warn`, `error` |
| `--help` | — | Show help and exit |

---

## Exit codes

| Code | Meaning |
|------|---------|
| `0` | Autotuner ran to completion |
| `1` | Argument / usage error |
| `2` | `.fal` file failed to load |
| `3` | Runtime error (e.g. unresolved dependency, type error) |

---

## Parameter type inference

`--param` values are parsed in this order:

| Value | Resolved type |
|-------|--------------|
| `true` / `false` | `bool` |
| Integer string | `int` (64-bit) |
| Float string | `float` (double) |
| Anything else | `string` |

Examples:

```bash
--param enabled=true           # bool
--param count=10               # int
--param voltage=0.5            # float
--param device_id=my_device    # string
```

---

## Examples

### Run a simple autotuner

```bash
falcon-run MyAutotuner my_autotuner.fal
```

### Run with initial parameters

```bash
falcon-run VoltageSweep sweep.fal \
  --param min_voltage=0.0 \
  --param max_voltage=1.5 \
  --param sweep_steps=100
```

### List autotuners in a file

```bash
falcon-run --list autotuners.fal
# Output:
# Loaded autotuners:
#   VoltageSweep
#   ChargeStability
```

### Load multiple files (e.g. shared imports)

```bash
falcon-run ChargeStability types.fal charge_stability.fal
```

### Enable verbose logging

```bash
falcon-run MyAutotuner file.fal --log-level debug
```

### Scripting: check exit code

```bash
falcon-run MyAutotuner file.fal && echo "OK" || echo "FAILED ($?)"
```

---

## Output format

On success, `falcon-run` prints the autotuner name and a numbered list of result values:

```
Autotuner 'VoltageSweep' completed.
Results (3):
  [0] 0.75
  [1] 1.2e-09
  [2] true
```

Result values map to the autotuner's persistent `params` in declaration order.

---

## Using ffimport-based autotuners

If your `.fal` file uses `ffimport` to bind a C++ hardware wrapper, `falcon-run` triggers the same compilation path that the engine uses programmatically: the wrapper is compiled with `clang++` into a cached `.so` in `.falcon/cache/` and loaded via `dlopen`. Ensure `clang++` is on your `PATH` and all `-I` include paths are reachable.

---

## Environment variables

| Variable | Effect |
|----------|--------|
| `LOG_LEVEL` | Overrides `--log-level` (same values) |
| `LOG_FILE` | Write log output to a file instead of stdout |
| `LOG_PATTERN` | Custom spdlog format pattern |
