# falcon/io

Standard I/O utilities for Falcon — capture and redirect stdout/stderr, plus direct write routines.

The `io` library provides:

1. **I/O Capture** — intercept text written to `stdout` or `stderr` so Falcon code can inspect it
2. **Write Routines** — direct output to stdout or stderr

The most common use-case is testing: wrap code under test in `BeginStdout` / `End` to capture output and assert its content.

---

## Installation

```fal
import "libs/io/io.fal";
```

---

## Quick Start

```fal
import "libs/io/io.fal";

autotuner CaptureOutputExample -> (int message_count) {
    message_count = 0;
    start -> test_capture;

    state test_capture {
        // Start capturing stdout
        Error err = io::BeginStdout();
        
        // Write some messages
        err = io::Println("Hello, World!");
        err = io::Println("Second message");
        
        // Capture and stop
        string output = io::End();
        // output now contains "Hello, World!\nSecond message\n"
        // and the text was also printed to the real stdout
        
        message_count = 2;
        -> done;
    }

    state done { terminal; }
}
```

---

## Writing Output

### `Print(string msg) -> (Error err)`

Write `msg` to stdout **without** a trailing newline.

```fal
Error err = io::Print("Loading");
err = io::Print(".");
err = io::Print(".");
err = io::Print(".");
// Output: "Loading..."
```

---

### `Println(string msg) -> (Error err)`

Write `msg` to stdout **with** a trailing newline.

```fal
Error err = io::Println("Initialization complete");
// Output: "Initialization complete\n"
```

---

### `Eprint(string msg) -> (Error err)`

Write `msg` to stderr **without** a trailing newline.

```fal
Error err = io::Eprint("Warning: ");
err = io::Eprint("High voltage detected");
```

---

### `Eprintln(string msg) -> (Error err)`

Write `msg` to stderr **with** a trailing newline.

```fal
Error err = io::Eprintln("Error: Device not responding");
// Output to stderr: "Error: Device not responding\n"
```

---

## Capturing Output

The capture API is **stateless and global** — there is a single process-wide capture buffer. Only one capture can be active at a time.

### `BeginStdout() -> (Error err)`

Start intercepting all bytes written to `stdout`.  While capture is active:

- Calls to `Println`, `Print`, and any C++ code writing to stdout are diverted to an internal buffer
- The output does **not** immediately appear on the terminal
- Only `WriteStdout` and `WriteStderr` bypass the capture

```fal
Error err = io::BeginStdout();
```

**Important:** Call `End()` before starting a new capture.

---

### `BeginStderr() -> (Error err)`

Start intercepting all bytes written to `stderr`.  Works the same as `BeginStdout` but for stderr.

```fal
Error err = io::BeginStderr();
```

---

### `End() -> (string captured)`

Stop the current capture session and return all accumulated text.

The `End` function:

1. Flushes C++ stream buffers into the capture buffer
2. Closes the internal pipe
3. **Tees** the captured bytes back to the real `stdout`/`stderr` so the output is not lost
4. Clears the capture state so `IsActive()` returns false
5. Returns the full captured text as a single `string`

```fal
string output = io::End();
// output contains everything written since BeginStdout/BeginStderr
// The output has also been printed to the real stdout/stderr
```

---

### `Peek() -> (string captured)`

Return all accumulated text in the capture buffer **without** stopping the capture.

Subsequent writes will still be captured and added to the buffer. The returned string is cumulative.

```fal
Error err = io::BeginStdout();
err = io::Println("First");
string so_far = io::Peek();  // "First\n"
err = io::Println("Second");
string so_far2 = io::Peek(); // "First\nSecond\n"
string final = io::End();    // "First\nSecond\n"
```

---

### `IsActive() -> (bool active)`

Returns `true` if a capture is currently in progress (after `BeginStdout` or `BeginStderr`, before `End`).

```fal
bool active = io::IsActive();  // false initially
Error err = io::BeginStdout();
active = io::IsActive();       // true
string _out = io::End();
active = io::IsActive();       // false again
```

---

## Writing Past Capture

When a capture is active, these routines write directly to the **real** stdout/stderr, bypassing the capture buffer. Useful for emitting diagnostics that should always be visible.

### `WriteStdout(string msg) -> (Error err)`

Write `msg` directly to the real `stdout`, even if `BeginStdout` is active.

```fal
Error err = io::BeginStdout();
err = io::Println("This is captured");
err = io::WriteStdout("-- CHECKPOINT --\n");  // Always visible
string output = io::End();
// output contains "This is captured\n"
// but "-- CHECKPOINT --\n" is NOT in output (went to real stdout)
```

---

### `WriteStderr(string msg) -> (Error err)`

Write `msg` directly to the real `stderr`, even if `BeginStderr` is active.

```fal
Error err = io::BeginStderr();
err = io::Eprintln("This is captured");
err = io::WriteStderr("!! ALERT !!\n");  // Always visible on real stderr
string output = io::End();
```

---

## Common Patterns

### Testing: Verify output content

```fal
import "libs/io/io.fal";
import "libs/strings/strings.fal";
import "libs/testing/testing.fal";

autotuner OutputTest -> (int passed, int failed) {
    passed = 0; failed = 0;
    start -> __init;

    state test_device_init_message (TestRunner runner, TestContext t) {
        Error err = io::BeginStdout();
        err = io::Println("Device initialized: Falcon X");
        string output = io::End();
        
        bool ok = strings::Contains(output, "Falcon X");
        err = t.ExpectTrue(ok, "Init message contains device name");
        -> __end(runner, t);
    }

    state test_error_on_stderr (TestRunner runner, TestContext t) {
        Error err = io::BeginStderr();
        err = io::Eprintln("Critical: Voltage out of range");
        string output = io::End();
        
        bool has_error = strings::Contains(output, "Critical");
        err = t.ExpectTrue(has_error, "Error message on stderr");
        -> __end(runner, t);
    }
}
```

### Logging: Inspect log output

```fal
import "libs/io/io.fal";
import "libs/log/log.fal";
import "libs/strings/strings.fal";

autotuner LogInspectionExample -> (int log_count) {
    log_count = 0;
    start -> inspect_logs;

    state inspect_logs {
        Error err = io::BeginStdout();
        
        err = log::Info("Starting sweep");
        err = log::Debug("Voltage = 1.0V");
        err = log::Info("Sweep complete");
        
        string output = io::End();
        
        // Count log lines
        int lines = 0;
        if (strings::Contains(output, "Starting sweep")) { lines = lines + 1; }
        if (strings::Contains(output, "Voltage = 1.0V")) { lines = lines + 1; }
        if (strings::Contains(output, "Sweep complete")) { lines = lines + 1; }
        
        log_count = lines;
        -> done;
    }

    state done { terminal; }
}
```

---

## Running the tests

```bash
falcon-test libs/io/tests/run_tests.fal
```

The test suite verifies:

- `Print` and `Println` write to captured stdout
- `Eprint` and `Eprintln` write to captured stderr
- `IsActive` correctly reflects capture state before/during/after capture
- Captured strings contain expected output
- `Peek` works without ending the capture
- `WriteStdout`/`WriteStderr` bypass capture when active

---

## Implementation Details

### Thread Safety

The underlying capture mechanism uses a single global pipe and mutex. Only one thread can have an active capture at a time. Attempting to call `BeginStdout` while another capture is active will fail (return error). This design simplifies implementation and matches typical Falcon usage (single-threaded state machines).

### File Descriptors

On Unix systems, `BeginStdout` and `BeginStderr` use `dup2` to redirect file descriptors 1 and 2 respectively. An internal pipe is created to hold captured data. When `End` is called, the original file descriptors are restored and the pipe is closed.

### Output Loss Prevention

The **tee** behavior of `End` ensures that captured output is not lost — it is both returned to the caller AND written to the real stdout/stderr. This allows testing while still observing the actual output.

---

## See Also

- [log — Structured logging](../log/README.md)
- [strings — String utilities and inspection](../strings/README.md)
- [testing — Test framework](../testing/README.md)
