# falcon/testing

A native Falcon testing library providing a fixture-aware, gtest-style test harness for writing and running tests entirely within the Falcon DSL. It binds a C++ backend via the FFI and is driven by the `falcon-test` CLI.

---

## Table of Contents

- [Overview](#overview)
- [How it works](#how-it-works)
- [Writing a test file](#writing-a-test-file)
  - [Minimal test (no fixture)](#minimal-test-no-fixture)
  - [With setup and teardown](#with-setup-and-teardown)
  - [Multi-step tests](#multi-step-tests)
  - [Multiple suites in one file](#multiple-suites-in-one-file)
- [Running tests](#running-tests)
- [TestContext API](#testcontext-api)
- [TestRunner API](#testrunner-api)
- [Terminal output format](#terminal-output-format)
- [Adding the dependency](#adding-the-dependency)
- [Self-tests](#self-tests)

---

## Overview

The library exposes two structs:

- **`TestContext`** — analogous to Go's `*testing.T`. One instance per test. Accumulates assertion failures without aborting execution so all failures within a single test are reported together.
- **`TestRunner`** — manages the test loop, index tracking, and all terminal output. Created and owned entirely by the generated harness; user code never constructs one directly.

The `falcon-test` CLI reads a user test file, injects the loop-management harness states into each suite autotuner, writes a temporary transformed `.fal` alongside the original (so `import` resolution continues to work), loads it through `AutotunerEngine`, and runs every discovered suite in sequence.

---

## How it works

A test suite is an `autotuner` block with the signature `-> (int passed, int failed)`. `falcon-test` scans for these, discovers every `state test_*` body inside each one, and synthesises five hidden harness states:

| Generated state | Role |
|---|---|
| `__init` | Registers all test names with `TestRunner`, prints the header, enters the loop |
| `__loop` | Calls `HasNext`; branches to `setup` (or `__begin`) or `__finish` |
| `__begin` | Calls `BeginCurrent` to create a fresh `TestContext`, then dispatches |
| `__dispatch` | One `if` branch per `test_*` name — routes to the correct user state |
| `__end` | Calls `EndCurrent`, prints pass/fail line, returns to `__loop` |
| `__finish` | Calls `PrintSummary`, writes `passed`/`failed` outputs, `terminal` |

Users see none of this machinery.

---

## Writing a test file

### Minimal test (no fixture)

```fal
import "/opt/falcon/libs/testing/testing.fal";

autotuner MathTests -> (int passed, int failed) {
    passed = 0; failed = 0;
    start -> __init;           // required entry point

    state test_addition (TestRunner runner, TestContext t) {
        Error err = t.ExpectIntEq(2 + 2, 4, "2+2=4");
        -> __end(runner, t);   // hand back to the harness
    }

    state test_subtraction (TestRunner runner, TestContext t) {
        Error err = t.ExpectIntEq(10 - 3, 7, "10-3=7");
        -> __end(runner, t);
    }
}
```

That is the complete file. No registration. No dispatch table. No loop logic.

### With setup and teardown

`setup` runs **before every test**. `teardown` runs **after every test**. Both are optional; omit either or both if you don't need them.

```fal
import "/opt/falcon/libs/testing/testing.fal";

autotuner DeviceTests -> (int passed, int failed) {
    passed = 0; failed = 0;
    int device_state = 0;          // shared environment variable
    start -> __init;

    // Runs before each test — MUST end with -> __begin(runner);
    state setup (TestRunner runner) {
        device_state = 42;          // reset to known state
        -> __begin(runner);
    }

    // Runs after each test — MUST end with -> __end(runner, t);
    state teardown (TestRunner runner, TestContext t) {
        Error err = t.Log("teardown: device_state was reset");
        -> __end(runner, t);
    }

    state test_reads_clean_state (TestRunner runner, TestContext t) {
        Error err = t.ExpectIntEq(device_state, 42, "setup ran: state is 42");
        -> teardown(runner, t);     // route through teardown before __end
    }

    state test_dirties_state (TestRunner runner, TestContext t) {
        device_state = 999;
        Error err = t.ExpectIntEq(device_state, 999, "dirtied to 999");
        -> teardown(runner, t);
    }

    state test_clean_again (TestRunner runner, TestContext t) {
        // setup ran again before this test, so device_state is 42 again
        Error err = t.ExpectIntEq(device_state, 42, "setup reset it to 42");
        -> teardown(runner, t);
    }
}
```

> **Convention summary**
>
> | State | Terminal transition |
> |---|---|
> | `setup` | `-> __begin(runner);` |
> | `teardown` | `-> __end(runner, t);` |
> | `test_*` (with teardown) | `-> teardown(runner, t);` |
> | `test_*` (no teardown) | `-> __end(runner, t);` |

### Multi-step tests

A test can span multiple states by threading `t` through intermediate states. Only the first state in the chain is registered and dispatched; continuing states are named freely but should **not** start with `test_` (otherwise they would be registered as independent tests).

```fal
    state test_device_sequence (TestRunner runner, TestContext t) {
        Error err = t.Log("phase 1: initialise");
        err = t.ExpectIntEq(device_state, 42, "phase 1: state clean");
        -> test_device_sequence_phase2(runner, t);
    }

    state test_device_sequence_phase2 (TestRunner runner, TestContext t) {
        Error err = t.Log("phase 2: operate");
        err = t.ExpectTrue(true, "phase 2: operation ok");
        -> teardown(runner, t);
    }
```

`falcon-test` detects that `test_device_sequence_phase2` is a **transition target** of another `test_*` state and therefore treats it as a continuation rather than an entry point. It is not registered or dispatched independently.

### Multiple suites in one file

Any number of suite autotuners can coexist in a single file. Each is run independently and gets its own header/summary block in the output. All imports are shared.

```fal
import "/opt/falcon/libs/testing/testing.fal";
import "my_device.fal";

autotuner UnitTests -> (int passed, int failed) {
    passed = 0; failed = 0;
    start -> __init;
    // ... unit test states ...
}

autotuner IntegrationTests -> (int passed, int failed) {
    passed = 0; failed = 0;
    start -> __init;
    // ... integration test states using MyDevice struct from my_device.fal ...
}
```

---

## Running tests

```bash
# Run a single test file
falcon-test my_tests.fal

# Run multiple files — all suites discovered, combined exit code
falcon-test unit_tests.fal integration_tests.fal

# Debug: print the generated .fal source without running
falcon-test my_tests.fal --dump

# Quieter engine output
falcon-test my_tests.fal --log-level error
```

`falcon-test` exits `0` if every test in every suite passed, `3` if any test failed.

---

## TestContext API

Write assertions against the `TestContext t` received by each test state. Assertions are **non-aborting** — execution continues after a failure and all failures within a single test are reported together.

| Routine | Description |
|---|---|
| `New(string name) -> (TestContext ctx)` | *(Used internally by the harness)* Create a named context |
| `Name -> (string name)` | Return the test name |
| `Passed -> (bool passed)` | `true` if no failures recorded |
| `FailureCount -> (int count)` | Number of recorded failures |
| `ExpectTrue(bool condition, string msg) -> (Error err)` | Fail if `condition` is `false` |
| `ExpectFalse(bool condition, string msg) -> (Error err)` | Fail if `condition` is `true` |
| `ExpectIntEq(int expected, int actual, string msg) -> (Error err)` | Fail if `expected != actual`; prints both values |
| `ExpectIntNe(int expected, int actual, string msg) -> (Error err)` | Fail if `expected == actual` |
| `ExpectIntLt(int a, int b, string msg) -> (Error err)` | Fail if `!(a < b)` |
| `ExpectIntGt(int a, int b, string msg) -> (Error err)` | Fail if `!(a > b)` |
| `ExpectFloatEq(float expected, float actual, float tol, string msg) -> (Error err)` | Fail if `|expected - actual| > tol` |
| `ExpectStrEq(string expected, string actual, string msg) -> (Error err)` | Fail if strings differ; prints both values |
| `ExpectStrNe(string expected, string actual, string msg) -> (Error err)` | Fail if strings are equal |
| `Fail(string msg) -> (Error err)` | Unconditionally record a failure |
| `Log(string msg) -> (Error err)` | Print an informational line immediately (`[ INFO     ]`) |

---

## TestRunner API

`TestRunner` is managed entirely by the generated harness. You should not need to call these methods directly, but they are documented for completeness.

| Routine | Description |
|---|---|
| `New(string suite_name) -> (TestRunner runner)` | Create a new runner for the named suite |
| `Register(string test_name) -> (Error err)` | Register a test name (called by `__init`) |
| `PrintHeader -> (Error err)` | Print the `[==========] Running N test(s)` header |
| `BeginCurrent -> (TestContext ctx)` | Print `[ RUN      ]`, return a fresh `TestContext` |
| `EndCurrent(TestContext ctx) -> (Error err)` | Print `[       OK ]` or `[  FAILED  ]`, advance index |
| `HasNext -> (bool has_next)` | `true` while tests remain |
| `CurrentName -> (string name)` | Name of the currently-running test |
| `PrintSummary -> (Error err)` | Print the final pass/fail summary block |
| `PassedCount -> (int count)` | Number of passing tests (available after run) |
| `FailedCount -> (int count)` | Number of failing tests (available after run) |

---

## Terminal output format

Output mirrors gtest's coloured format:

```
falcon-test: my_tests.fal
  suite [MathTests]  2 test(s)

[==========] Running 2 test(s) from MathTests
[----------] Global test environment set-up.
[ RUN      ] MathTests.test_addition
[       OK ] MathTests.test_addition
[ RUN      ] MathTests.test_fail_example
             ✗ 1 != 2 — mismatch should be reported (expected=1 actual=2)
[  FAILED  ] MathTests.test_fail_example

[==========] 2 test(s) from MathTests
[----------] Global test environment tear-down.
[  PASSED  ] test_addition
[  FAILED  ] test_fail_example
             ✗ 1 != 2 — mismatch should be reported (expected=1 actual=2)
[----------]
[  PASSED  ] 1 test(s)
[  FAILED  ] 1 test(s)
[==========] 2 test(s) ran.
```

`[ INFO     ]` lines from `t.Log(...)` appear inline as a test runs:

```
[ RUN      ] DeviceTests.test_device_sequence
[ INFO     ] test_device_sequence: phase 1: initialise
[ INFO     ] test_device_sequence: phase 2: operate
[       OK ] DeviceTests.test_device_sequence
```

---

## Adding the dependency

In your `falcon.yml`:

```yaml
dependencies:
  - testing
```

Then import in your `.fal` file:

```fal
import "/opt/falcon/libs/testing/testing.fal";
```

---

## Self-tests

The library ships two test files in `libs/testing/tests/`:

| File | Purpose |
|---|---|
| `run_tests.fal` | All assertions pass — verifies every assertion routine works correctly |
| `error_detection.fal` | Intentional failures — verifies the framework **detects and reports** failures; expected output is 3 passed / 3 failed in `ErrorDetection` and 1 passed / 2 failed in `MultipleFailuresPerTest` |

```bash
# Run the passing suite (exit code 0)
falcon-test libs/testing/tests/run_tests.fal

# Run the failure-detection suite (exit code 3 — failures are expected and correct)
falcon-test libs/testing/tests/error_detection.fal

# Run both together
falcon-test libs/testing/tests/run_tests.fal libs/testing/tests/error_detection.fal
```

To verify the error-detection suite is working correctly in CI, check that `failed` equals the expected count rather than checking the exit code:

```bash
# The error_detection suite should report exactly 5 total failures (3 + 2).
# Exit code 3 is expected and correct — it means the framework is catching them.
falcon-test libs/testing/tests/error_detection.fal || true
```
