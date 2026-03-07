# falcon/falconCore/math/arrays/controlArray

Falcon binding for `falcon_core::math::arrays::ControlArray` — a typed array with control semantics (monotone alignment and principle dimension).

---

## Installation

```fal
import "libs/falconCore/math/arrays/controlArray/controlArray.fal";
```

---

## Overview

`ControlArray` extends `FArray<double>` with control-axis semantics.  On construction it computes the `IncreasingAlignment` of the data (the direction of monotone change) and records the `principle_dimension` (the axis along which the array is monotone).  All arithmetic operations return a new `ControlArray`.

---

## API

```fal
// Constructor — wrap an existing FArray
routine New(arr: FArray) -> (ControlArray carray)

// Arithmetic (element-wise; overloaded for ControlArray, FArray, int, float)
routine Add    (other)     -> (ControlArray carray)
routine Minus  (other)     -> (ControlArray carray)
routine Negate ()          -> (ControlArray carray)
routine Times  (factor)    -> (ControlArray carray)
routine Divides(factor)    -> (ControlArray carray)
routine Power  (exponent)  -> (ControlArray carray)
routine Abs    ()          -> (ControlArray carray)

// Reduction
routine Min() -> (float min)
routine Max() -> (float max)

// Control metadata
routine PrincipleDimension() -> (int dim)
routine Alignment()          -> (IncreasingAlignment alignment)

// Shape
routine Reshape(shape: Array<int>) -> (ControlArray carray)
routine Flip   (axis: int)         -> (ControlArray carray)

// Gradient
routine Gradient    (axis: int) -> (FArray gradient)
routine FullGradient()          -> (Array<FArray> gradients)

// Equality
routine Equal   (other: ControlArray) -> (bool equal)
routine NotEqual(other: ControlArray) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (ControlArray carray)
```

---

## Example

```fal
import "libs/falconCore/math/arrays/controlArray/controlArray.fal";
import "libs/falconCore/generic/farray/farray.fal";

// Construct from an existing FArray
farray::FArray fa = farray::FArray.FromJSON(json);
ControlArray   ca = ControlArray.New(fa);

// Arithmetic
ControlArray scaled  = ca.Times(2.0);
ControlArray shifted = ca.Add(1.0);
ControlArray power   = ca.Power(2.0);

// Metadata
int  dim  = ca.PrincipleDimension();
increasingAlignment::IncreasingAlignment al = ca.Alignment();

// Serialisation round-trip
string json2        = ca.ToJSON();
ControlArray ca_rt  = ControlArray.FromJSON(json2);
bool same           = ca.Equal(ca_rt);
```
