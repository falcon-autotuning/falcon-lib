# falcon/collections/array

Generic ordered-list type for Falcon.

`Array<T>` is a generic struct backed by an FFI C++ implementation.

---

## Installation

```fal
import "libs/collections/array/array.fal";
```

In your `falcon.yml`:

```yaml
dependencies:
  - array
```

---

## Type

```fal
struct Array <T> { ... }
```

`T` may be any primitive (`int`, `float`, `bool`, `string`) or struct type.

---

## Routines

### `New() -> (Array<T> arr)`
Creates a new, empty array.
```fal
Array<int> nums = Array.New();
```

### `Size() -> (int size)`
Returns the number of elements.
```fal
int n = nums.Size();
```

### `IsEmpty() -> (bool empty)`
```fal
bool e = nums.IsEmpty();
```

### `GetIndex(int index) -> (T value)`
Returns the element at position `index` (0-based).  Also invoked by the
`arr[i]` index syntax.
```fal
int v = nums.GetIndex(0);
int v = nums[0];           // equivalent
```

### `SetIndex(int index, T value) -> ()`
Replaces the element at position `index`.
```fal
nums.SetIndex(0, 42);
```

### `PushBack(T value) -> ()`
Appends to the end.
```fal
nums.PushBack(99);
```

### `PopBack() -> (T value)`
Removes and returns the last element.
```fal
int last = nums.PopBack();
```

### `Insert(int index, T value) -> ()`
Inserts before position `index`.  `index == Size()` appends.
```fal
nums.Insert(0, 10);   // prepend
```

### `Erase(int index) -> ()`
Removes the element at `index`.
```fal
nums.Erase(1);
```

### `Clear() -> ()`
Removes all elements.
```fal
nums.Clear();
```

### `Contains(T value) -> (bool found)`
```fal
bool has = nums.Contains(42);
```

### `IndexOf(T value) -> (int index)`
Returns the index of the first occurrence, or `-1`.
```fal
int pos = nums.IndexOf(42);
```

---

## Index syntax

`arr[i]` desugars to `arr.GetIndex(i)` in the interpreter.

---

## Syntax note

`Array<T>` uses the standard generic struct syntax — no `uses` or `requires`
clause is needed. The type parameter is simply declared in angle brackets:

```fal
struct Array <T> { ... }          // definition
Array<int>    nums = Array.New(); // monomorphised use
Array<string> strs = Array.New();
```

---

## Running the tests

```bash
falcon-test libs/collections/array/tests/run_tests.fal
```

---

## Full example

```fal
import "libs/collections/array/array.fal";

autotuner ArrayDemo (int a, int b, int c) -> (int sum) {
    sum = 0;
    start -> run;
    state run {
        Array<int> nums = Array.New();
        nums.PushBack(a);
        nums.PushBack(b);
        nums.PushBack(c);
        sum = nums[0] + nums[1] + nums[2];
        terminal;
    }
}
```
