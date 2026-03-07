# falcon/falconCore/math/axes

Falcon generic ordered-list struct `Axes<T>` — a thin, type-safe wrapper around `Array<T>` with a consistent axis-oriented API.

---

## Installation

```fal
import "libs/falconCore/math/axes/axes.fal";
```

---

## Overview

`Axes<T>` is a FAL struct (not a C++ opaque) that wraps `Array<T>` and exposes a named set of operations suited for ordered axis values.  All core operations (`New`, `PushBack`, `GetIndex`, etc.) are implemented as inline FAL routines delegating to the underlying `Array<T>`.

---

## API

```fal
// Construction
routine New() -> (Axes<T> axes)

// Capacity
routine Size   ()         -> (int size)
routine IsEmpty()         -> (bool empty)

// Element access
routine GetIndex(int index)          -> (T value)
routine SetIndex(int index, T value) -> ()

// Mutation
routine PushBack(T value)            -> ()
routine PopBack ()                   -> (T value)
routine Insert  (int index, T value) -> ()
routine Erase   (int index)          -> ()
routine Clear   ()                   -> ()

// Search
routine Contains(T value)  -> (bool found)
routine IndexOf (T value)  -> (int index)
```

---

## Example

```fal
import "libs/falconCore/math/axes/axes.fal";

Axes<float> ax = Axes<float>.New();
ax.PushBack(1.0);
ax.PushBack(2.0);
ax.PushBack(3.0);

int   sz    = ax.Size();           // 3
bool  empty = ax.IsEmpty();        // false
float first = ax.GetIndex(0);      // 1.0
bool  found = ax.Contains(2.0);    // true
int   idx   = ax.IndexOf(3.0);     // 2

ax.SetIndex(1, 9.0);
float mid   = ax.GetIndex(1);      // 9.0

float last  = ax.PopBack();        // 3.0
ax.Insert(0, 0.5);
ax.Erase(0);
ax.Clear();
bool  now_empty = ax.IsEmpty();    // true
```
