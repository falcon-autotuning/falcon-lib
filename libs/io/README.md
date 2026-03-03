# falcon/io

Standard I/O utilities for Falcon — stdout/stderr capture and plain write helpers.

---

## Installation

```fal
import "libs/io/io.fal";
```

---

## Overview

The `io` library provides two things:

1. **`IOCapture`** — a struct that lets you intercept (and optionally inspect) text written to `stdout` or `stderr` from inside a Falcon autotuner.
2. **Top-level routines** — `Print`, `Println`, `Eprint`, `Eprintln` for direct writing.

The most common use-case is testing: wrap the code under test in a `BeginStdout` / `End` pair and assert that the captured string contains the expected output.

---

## IOCapture

### `IOCapture.New() -> (IOCapture cap)`

Create a new capture handle.  The handle is idle until `BeginStdout` or `BeginStderr` is called.

```fal
IOCapture cap = IOCapture.New();
```

---

### `cap.BeginStdout() -> (Error err)`

Start intercepting all bytes written to `stdout`.  While capture is active, writes from any Falcon routine (including `Println`, `log.Info`, etc.) are diverted into an internal pipe rather than appearing on the terminal immediately.

```fal
Error err = cap.BeginStdout();
```

> **One capture at a time.** Only call `BeginStdout` once per handle; call `End` before starting a new capture.

---

### `cap.BeginStderr() -> (Error err)`

Same as `BeginStdout` but for `stderr`.

```fal
Error err = cap.BeginStderr();
```

---

### `cap.End() -> (string captured)`

Ends the current capture session:

1. Flushes C++ stream buffers into the pipe.
2. Reads everything out of the pipe.
3. **Tees** the captured bytes back to the real `stdout`/`stderr` so the output is not lost.
4. Returns the full captured text as a single `string`.

```fal
string output = cap.End();
// output contains everything written since BeginStdout/BeginStderr
```

---

### `cap.Peek() -> (string captured)`

Drain whatever has accumulated in the pipe so far **without** ending the capture.  Subsequent writes will still be captured.  The returned string is cumulative.

```fal
string so_far = cap.Peek();
```

---

### `cap.IsActive() -> (bool active)`

Returns `true` while a capture is in progress.

```fal
bool active = cap.IsActive();
```

---

### `cap.WriteStdout(string msg) -> (Error err)`

Write `msg` directly to the **real** `stdout`, bypassing any active capture pipe.  Useful for emitting test-harness diagnostics that should always be visible.

```fal
Error err = cap.WriteStdout("-- checkpoint reached --\n");
```

---

### `cap.WriteStderr(string msg) -> (Error err)`

Same as `WriteStdout` but targets `stderr`.

---

## Top-level routines

| Routine | Description |
|---------|-------------|
| `Print(string msg) -> (Error err)` | Write `msg` to stdout (no newline) |
| `Println(string msg) -> (Error err)` | Write `msg` + `\n` to stdout |
| `Eprint(string msg) -> (Error err)` | Write `msg` to stderr (no newline) |
| `Eprintln(string msg) -> (Error err)` | Write `msg` + `\n` to stderr |

```fal
import "libs/io/io.fal";

autotuner Hello -> (int done) {
    done = 0;
    start -> greet;

    state greet {
        Error err = Println("Hello, Falcon!");
        done = 1;
        terminal;
    }
}
```

---

## Full capture example

```fal
import "libs/io/io.fal";
import "libs/log/log.fal";

autotuner CaptureDemo -> (int done) {
    done = 0;
    start -> run;

    state run {
        IOCapture cap = IOCapture.New();
        Error err = cap.BeginStdout();

        err = Println("line one");
        err = Println("line two");

        string output = cap.End();
        // output == "line one\nline two\n"
        // Both lines also appeared on the real stdout.

        done = 1;
        terminal;
    }
}
```

---

## Notes

- Only **one** capture may be active per `IOCapture` handle at a time.
- The library uses POSIX `pipe` / `dup2` under the hood; it is compatible with Linux and macOS.
- ANSI escape codes (colour) written by the testing framework are included in captured output.  Use the `strings` library's `Contains` routine if you need to match around them, or strip them first with `StripAnsi`.
