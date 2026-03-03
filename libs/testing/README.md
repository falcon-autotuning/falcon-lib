# falcon/testing

A native Falcon testing library that provides a lightweight test harness for writing and running tests in the Falcon DSL. It follows the same `ffimport` FFI pattern as `libs/log/`, binding a C++ backend to Falcon via the established foreign-function interface mechanism.

## Overview

The library exposes two structs:

- **`TestContext`** — analogous to Go's `*testing.T`. Create one per test, pass it through states, and check `Passed()` at the end.
- **`TestSuite`** — groups multiple `TestContext` results for summary reporting.

There is no class inheritance involved. The fixture pattern is simulated by passing a `TestContext` struct explicitly between states.

## TestContext API

| Routine | Description |
|---|---|
| `New(string name) -> (TestContext ctx)` | Create a new test context with the given name |
| `Name -> (string name)` | Return the test name |
| `Passed -> (bool passed)` | Return `true` if no failures have been recorded |
| `FailureCount -> (int count)` | Return the number of recorded failures |
| `ExpectTrue(bool condition, string msg) -> (Error err)` | Fail if `condition` is `false` |
| `ExpectFalse(bool condition, string msg) -> (Error err)` | Fail if `condition` is `true` |
| `ExpectIntEq(int expected, int actual, string msg) -> (Error err)` | Fail if `expected != actual` |
| `ExpectIntNe(int expected, int actual, string msg) -> (Error err)` | Fail if `expected == actual` |
| `ExpectIntLt(int a, int b, string msg) -> (Error err)` | Fail if `!(a < b)` |
| `ExpectIntGt(int a, int b, string msg) -> (Error err)` | Fail if `!(a > b)` |
| `ExpectFloatEq(float expected, float actual, float tol, string msg) -> (Error err)` | Fail if `|expected - actual| > tol` |
| `ExpectStrEq(string expected, string actual, string msg) -> (Error err)` | Fail if strings differ |
| `ExpectStrNe(string expected, string actual, string msg) -> (Error err)` | Fail if strings are equal |
| `Fail(string msg) -> (Error err)` | Unconditionally record a failure |
| `Log(string msg) -> (Error err)` | Emit a log message (printed immediately to stdout) |

## TestSuite API

| Routine | Description |
|---|---|
| `New(string suite_name) -> (TestSuite suite)` | Create a new test suite |
| `AddResult(TestContext ctx) -> (Error err)` | Add a completed test context to the suite |
| `TotalCount -> (int count)` | Total number of tests added |
| `PassedCount -> (int count)` | Number of passing tests |
| `FailedCount -> (int count)` | Number of failing tests |
| `PrintSummary -> (Error err)` | Print a formatted summary to stdout |

## Fixture Pattern

Since Falcon structs cannot use inheritance, the fixture pattern is simulated by passing a `TestContext` through multiple states:

```fal
autotuner MyTest -> (bool passed) {
  passed = false;

  start -> setup;

  state setup {
    TestContext ctx = TestContext.New("my-test");
    ctx.Log("setup complete");
    -> step_one(ctx);
  }

  state step_one (TestContext ctx) {
    ctx.ExpectIntEq(1, 1, "step 1");
    -> step_two(ctx);
  }

  state step_two (TestContext ctx) {
    ctx.ExpectStrEq("falcon", "falcon", "step 2");
    -> teardown(ctx);
  }

  state teardown (TestContext ctx) {
    ctx.Log("teardown");
    passed = ctx.Passed();
    -> done;
  }

  state done { terminal; }
}
```

## Adding the dependency

In your `falcon.yml`:

```yaml
dependencies:
  - testing
```

Then import in your `.fal` file:

```fal
import "libs/testing/testing.fal"
```

## Running the self-tests

```bash
cd libs/testing/tests
/opt/falcon/bin/falcon-interp test_self.fal
```

## Example test file

```fal
import "libs/testing/testing.fal"

autotuner TestMyFeature -> (bool passed) {
  passed = false;

  start -> run;

  state run {
    TestContext t = TestContext.New("my-feature");
    t.ExpectIntEq(2, 1 + 1, "addition");
    t.ExpectStrEq("hello", "hello", "strings");
    passed = t.Passed();
    -> done;
  }

  state done { terminal; }
}
```
