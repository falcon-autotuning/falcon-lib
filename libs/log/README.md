# falcon/log

Structured logging for Falcon autotuners — console and file output with configurable verbosity, patterns, and log levels.

The `log` library binds [spdlog](https://github.com/gabime/spdlog), a fast, header-only C++ logging library, and provides four severity levels for diagnostic and operational logging.

---

## Installation

```fal
import "libs/log/log.fal";
```

---

## Quick Start

```fal
import "libs/log/log.fal";

autotuner MyAutotuner (float param) -> (float result) {
    result = 0.0;
    start -> run;

    state run {
        Error err = log::Info("Starting parameter sweep");
        err = log::Debug("param value: " + strings::FromDouble(param));
        
        result = param * 2.0;
        
        err = log::Info("Sweep complete");
        -> done;
    }

    state done { terminal; }
}
```

---

## API Reference

### `Info(string message) -> (Error err)`

Log an informational message at `info` level.  This is the standard level for operational events and milestone announcements.

```fal
Error err = log::Info("Device initialized successfully");
```

---

### `Debug(string message) -> (Error err)`

Log a diagnostic message at `debug` level.  Use this for detailed tracing that is typically hidden in production.

```fal
Error err = log::Debug("Sweep parameter: voltage=0.75V, step=0.01V");
```

---

### `Warn(string message) -> (Error err)`

Log a warning message at `warn` level.  Use this to report unusual but non-fatal conditions.

```fal
Error err = log::Warn("Device calibration is stale (>24h old)");
```

---

### `Err(string message) -> (Error err)`

Log an error message at `error` level.  Use this to report failures that should be investigated but do not prevent execution.

```fal
Error err = log::Err("Measurement out of expected range: 2.5mA (expected <1mA)");
```

---

## Log Levels

| Level | Environment Variable | Visibility | Use Case |
|-------|----------------------|------------|----------|
| `trace` | `LOG_LEVEL=trace` | Very verbose | Low-level debugging (rarely used) |
| `debug` | `LOG_LEVEL=debug` | Verbose | Development and diagnostics |
| `info` | `LOG_LEVEL=info` (default) | Standard | Operational events, milestones |
| `warn` | `LOG_LEVEL=warn` | Important | Unusual conditions, caution alerts |
| `error` | `LOG_LEVEL=error` | Critical | Failures and errors |
| `off` | `LOG_LEVEL=off` | Silent | Disable all logging |

---

## Configuration

The logger is configured entirely via environment variables. Set these before running `falcon-run`:

### `LOG_LEVEL`

Controls which messages are emitted.

```bash
export LOG_LEVEL=debug
falcon-run MyAutotuner my_autotuner.fal
```

Default: `info`

---

### `LOG_FILE`

If set, enables dual-sink output: logs are written to both a rotating file and the console.  Without this variable, logs go to console only.

```bash
export LOG_FILE=/tmp/falcon-autotuner.log
falcon-run MyAutotuner my_autotuner.fal

# Logs appear in /tmp/falcon-autotuner.log (rotated at 10MB, keeping 3 files)
# Warnings and errors also appear on stderr
```

Default: not set (console-only mode)

---

### `LOG_PATTERN`

Custom spdlog format string.  See [spdlog format reference](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting) for syntax.

Common placeholders:

- `%Y-%m-%d %H:%M:%S.%f` — ISO 8601 timestamp with milliseconds
- `%l` — log level (trace, debug, info, warn, error, critical)
- `%v` — message body
- `%^...%$` — color range (wraps text in ANSI color codes)

```bash
export LOG_PATTERN="[%H:%M:%S] [%^%-8l%$] %v"
falcon-run MyAutotuner my_autotuner.fal
```

Output:

```
[14:32:01] [info    ] Device initialized
[14:32:01] [debug   ] Setting voltage to 0.75V
[14:32:02] [warn    ] High current detected: 2.1mA
```

Default:

```
[%Y-%m-%d %H:%M:%S.%f] [%^%l%$] %v
```

---

## File Rotation

When `LOG_FILE` is set:

- Logs are written to a rotating file sink (max 10 MB per file)
- Up to 3 rotated backup files are retained
- Older files are automatically deleted
- The current file is always named as specified by `LOG_FILE`
- Errors and warnings also appear on `stderr` for immediate visibility

Example with 10 MB files:

```
/tmp/falcon-autotuner.log      (current, <10 MB)
/tmp/falcon-autotuner.log.1    (previous, ~10 MB)
/tmp/falcon-autotuner.log.2    (older, ~10 MB)
```

---

## Examples

### Development: Verbose console output

```bash
export LOG_LEVEL=debug
falcon-run VoltageSweeep sweep.fal --param min=0.0 --param max=1.5
```

### Production: File + console errors

```bash
export LOG_LEVEL=info
export LOG_FILE=/var/log/falcon/autotuner.log
falcon-run ChargeStability sweep.fal
```

### Custom timestamp format

```bash
export LOG_PATTERN="[%T] %l: %v"   # [HH:MM:SS] level: message
falcon-run MyAutotuner my_autotuner.fal
```

### Capture only warnings and above

```bash
export LOG_LEVEL=warn
falcon-run MyAutotuner my_autotuner.fal
```

---

## Combining with Other Libraries

The `log` library works well with `io` for capturing output:

```fal
import "libs/log/log.fal";
import "libs/io/io.fal";

autotuner CaptureLogsExample -> (int captured_count) {
    captured_count = 0;
    start -> test_logging;

    state test_logging {
        // Start capturing stdout
        Error err = io::BeginStdout();
        
        // Emit some log messages
        err = log::Info("First message");
        err = log::Debug("Second message");
        err = log::Warn("Third message");
        
        // Capture all output
        string output = io::End();
        
        // Count messages
        int count = 0;
        if (strings::Contains(output, "First")) { count = count + 1; }
        if (strings::Contains(output, "Second")) { count = count + 1; }
        if (strings::Contains(output, "Third")) { count = count + 1; }
        
        captured_count = count;
        -> done;
    }

    state done { terminal; }
}
```

---

## Running the tests

```bash
falcon-test libs/log/tests/run_tests.fal
```

The test suite verifies:

- All four log levels (`Info`, `Debug`, `Warn`, `Err`) produce output
- Messages appear in captured stdout with correct content
- Level tags are present in the formatted output
- ANSI color codes are included (verified with `StripAnsi`)

---

## Implementation Details

### Thread Safety

The underlying spdlog logger uses thread-safe sinks (stdout/stderr and rotating file sinks are all multi-threaded safe). All four routines are safe to call concurrently from multiple Falcon states.

### Error Handling

Each routine returns an `Error`. Currently these are always `nil` (success). Future versions may return meaningful errors for I/O failures (e.g., disk full when writing to `LOG_FILE`).

### Performance

Spdlog is highly optimized for low-latency logging. The default configuration is suitable for real-time quantum device control with minimal overhead.

---

## See Also

- [io — I/O capture and output](../io/README.md)
- [strings — String utilities](../strings/README.md)
- [testing — Test framework](../testing/README.md)
- [spdlog documentation](https://github.com/gabime/spdlog)
