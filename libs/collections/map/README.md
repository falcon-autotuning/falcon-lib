# falcon/collections/map

Generic key-value store for Falcon.

`Map<K, V>` is a two-parameter generic struct backed by an FFI C++ `std::map`.

---

## Installation

```fal
import "libs/collections/map/map.fal";
```

In your `falcon.yml`:

```yaml
dependencies:
  - map
```

---

## Type

```fal
struct Map <K, V> { ... }
```

`K` must be a primitive type (`int`, `float`, `bool`, or `string`).
`V` may be any primitive or struct type.

---

## Routines

### `New() -> (Map<K,V> m)`
Creates a new, empty map.
```fal
Map<string, int> scores = Map.New();
```

### `Size() -> (int size)`
Returns the number of key-value pairs.

### `IsEmpty() -> (bool empty)`

### `Get(K key) -> (V value)`
Returns the value for `key`.  Throws a runtime error if `key` is absent.
```fal
int v = scores.Get("alice");
```

### `GetOr(K key, V default_value) -> (V value)`
Returns the value for `key`, or `default_value` if absent.
```fal
int v = scores.GetOr("bob", 0);
```

### `Set(K key, V value) -> ()`
Inserts or overwrites.
```fal
scores.Set("alice", 100);
```

### `Remove(K key) -> (bool removed)`
Removes an entry.  Returns `true` if the key existed.
```fal
bool was_there = scores.Remove("alice");
```

### `Clear() -> ()`
Removes all entries.

### `Contains(K key) -> (bool found)`
```fal
bool has = scores.Contains("alice");
```

### `Keys() -> (Array<K> keys)`
Returns an `Array<K>` of all keys in unspecified order.

### `Values() -> (Array<V> values)`
Returns an `Array<V>` of all values in unspecified order.

---

## Syntax note

`Map<K, V>` uses the standard two-parameter generic struct syntax.
No `uses` or `requires` clause is needed:

```fal
struct Map <K, V> { ... }              // definition
Map<string, int> m = Map.New();        // monomorphised use
Map<int,    int> n = Map.New();
```

---

## Running the tests

```bash
falcon-test libs/collections/map/tests/run_tests.fal
```

---

## Full example

```fal
import "libs/collections/map/map.fal";
import "libs/collections/array/array.fal";

autotuner WordCount (string a, string b, string c) -> (int distinct) {
    distinct = 0;
    start -> run;
    state run {
        Map<string, int> wc = Map.New();
        wc.Set(a, 1);
        wc.Set(b, 1);
        wc.Set(c, 1);
        distinct = wc.Size();
        terminal;
    }
}
```
