# falcon/strings

String utilities and regex support for Falcon.

---

## Installation

```fal
import "libs/strings/strings.fal";
```

---

## Predicate routines

### `Contains(string s, string sub) -> (bool found)`

Returns `true` if `sub` appears anywhere inside `s`.

```fal
bool ok = Contains("hello world", "world");  // true
```

---

### `StartsWith(string s, string prefix) -> (bool found)`

```fal
bool ok = StartsWith("falcon-lib", "falcon");  // true
```

---

### `EndsWith(string s, string suffix) -> (bool found)`

```fal
bool ok = EndsWith("run_tests.fal", ".fal");  // true
```

---

### `Length(string s) -> (int len)`

Returns the number of **bytes** in `s` (not Unicode code-points).

```fal
int n = Length("hello");  // 5
```

---

### `IsEmpty(string s) -> (bool empty)`

```fal
bool e = IsEmpty("");   // true
bool e = IsEmpty("x");  // false
```

---

## Transformation routines

### `ToUpper(string s) -> (string result)`

```fal
string up = ToUpper("falcon");  // "FALCON"
```

---

### `ToLower(string s) -> (string result)`

```fal
string lo = ToLower("FALCON");  // "falcon"
```

---

### `TrimSpace(string s) -> (string result)`

Strips leading and trailing whitespace (space, `\t`, `\r`, `\n`, `\f`, `\v`).

```fal
string t = TrimSpace("  hello\n");  // "hello"
```

---

### `Replace(string s, string from, string to) -> (string result)`

Replaces the **first** occurrence of `from` with `to`.

```fal
string r = Replace("aababc", "ab", "X");  // "aXabc"
```

---

### `ReplaceAll(string s, string from, string to) -> (string result)`

Replaces **all** non-overlapping occurrences.

```fal
string r = ReplaceAll("aababc", "ab", "X");  // "aXXc"
```

---

### `Concat(string a, string b) -> (string result)`

```fal
string s = Concat("foo", "bar");  // "foobar"
```

---

### `Substring(string s, int start, int length) -> (string result)`

Extracts `length` bytes starting at byte offset `start` (0-based).

```fal
string s = Substring("hello world", 6, 5);  // "world"
```

---

### `IndexOf(string s, string sub) -> (int index)`

Returns the byte offset of the first occurrence of `sub`, or `-1` if not found.

```fal
int i = IndexOf("hello world", "world");  // 6
int i = IndexOf("hello world", "xyz");    // -1
```

---

### `StripAnsi(string s) -> (string result)`

Removes ANSI/VT100 escape sequences such as colour codes.  Useful when
capturing output from the testing framework and matching against plain text.

```fal
string plain = StripAnsi("\033[32m[  PASSED  ]\033[0m MyTest");
// plain == "[  PASSED  ] MyTest"
```

---

## Regex

### `RegexMatch` struct

#### `RegexMatch.New(string pattern, string text) -> (RegexMatch m)`

Run `pattern` against `text` using ECMAScript regex syntax.  Returns a match object.

```fal
RegexMatch m = RegexMatch.New("(\\w+)@(\\w+)", "user@example");
```

#### `m.Matched() -> (bool matched)`

`true` if the pattern found a match anywhere in `text`.

#### `m.Full() -> (string full_match)`

The full matched substring.

```fal
string full = m.Full();  // "user@example"
```

#### `m.Group(int n) -> (string group)`

Capture group `n` (1-based).  Returns `""` if `n` is out of range.

```fal
string user   = m.Group(1);  // "user"
string domain = m.Group(2);  // "example"
```

#### `m.GroupCount() -> (int count)`

Number of capture groups (not counting group 0).

```fal
int gc = m.GroupCount();  // 2
```

---

### `RegexTest(string pattern, string text) -> (bool matched)`

Quick boolean check — no object allocation.

```fal
bool ok = RegexTest("\\d+", "abc123");  // true
bool ok = RegexTest("\\d+", "abcdef");  // false
```

---

### `RegexReplace(string s, string pattern, string replacement) -> (string result)`

Replaces all non-overlapping matches of `pattern` in `s` with `replacement`.
Standard ECMAScript back-references (`$1`, `$2`, …) are supported.

```fal
string r = RegexReplace("2026-03-03", "(\\d{4})-(\\d{2})-(\\d{2})", "$3/$2/$1");
// r == "03/03/2026"
```

---

## Full example

```fal
import "libs/strings/strings.fal";

autotuner StringDemo -> (int done) {
    done = 0;
    start -> run;

    state run {
        bool has_log = Contains("[INFO]  hello", "[INFO]");
        string upper = ToUpper("falcon");
        bool ok      = RegexTest("\\d{4}-\\d{2}-\\d{2}", "today is 2026-03-03");
        done = 1;
        terminal;
    }
}
```
