# falcon/falconCore/math/arrays/increasingAlignment

Falcon binding for `falcon_core::math::arrays::IncreasingAlignment` — a three-state alignment indicator (increasing, decreasing, or no trend).

---

## Installation

```fal
import "libs/falconCore/math/arrays/increasingAlignment/increasingAlignment.fal";
```

---

## Overview

`IncreasingAlignment` encodes the directional trend of an array domain using three internal states: `1` (increasing), `-1` (decreasing), and `0` (no trend).  Construction from a `bool` maps `true` → increasing and `false` → decreasing.

---

## API

```fal
// Constructor
routine New(bool alignment) -> (IncreasingAlignment inc)

// Accessor
// Returns true when the alignment is increasing (internal value > 0).
routine Alignment() -> (bool alignment)

// Equality
routine Equal   (IncreasingAlignment other) -> (bool equal)
routine NotEqual(IncreasingAlignment other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (IncreasingAlignment farray)
```

---

## Example

```fal
import "libs/falconCore/math/arrays/increasingAlignment/increasingAlignment.fal";

IncreasingAlignment inc = IncreasingAlignment.New(true);
IncreasingAlignment dec = IncreasingAlignment.New(false);
bool a  = inc.Alignment();       // true  (increasing)
bool b  = dec.Alignment();       // false (decreasing)
bool eq = inc.Equal(inc);        // true
bool ne = inc.NotEqual(dec);     // true
string json = inc.ToJSON();
IncreasingAlignment rt = IncreasingAlignment.FromJSON(json);
bool same = inc.Equal(rt);       // true
```
