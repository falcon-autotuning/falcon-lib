# falcon/falconCore/math/arrays/measuredArray

Falcon binding for `falcon_core::math::arrays::MeasuredArray` — a typed array for measured (sampled) data, providing element-wise arithmetic and gradient computation.

---

## Installation

```fal
import "libs/falconCore/math/arrays/measuredArray/measuredArray.fal";
```

---

## Overview

`MeasuredArray` extends `FArray<double>` with semantics for measured (i.e. experimental or sampled) data.  It supports the same element-wise arithmetic as `FArray` but does not carry control-axis metadata (`PrincipleDimension` / `Alignment`).  Equality is tested against any `FArray` (including other `MeasuredArray` values).

---

## API

```fal
// Constructor — wrap an existing FArray
routine New(arr: FArray) -> (MeasuredArray carray)

// Arithmetic (element-wise; overloaded for MeasuredArray, FArray, int, float)
routine Add    (other)     -> (MeasuredArray carray)
routine Minus  (other)     -> (MeasuredArray carray)
routine Negate ()          -> (MeasuredArray carray)
routine Times  (factor)    -> (MeasuredArray carray)
routine Divides(factor)    -> (MeasuredArray carray)
routine Power  (exponent)  -> (MeasuredArray carray)
routine Abs    ()          -> (MeasuredArray carray)

// Reduction
routine Min() -> (float min)
routine Max() -> (float max)

// Shape
routine Reshape(shape: Array<int>) -> (MeasuredArray carray)
routine Flip   (axis: int)         -> (MeasuredArray carray)

// Gradient
routine Gradient    (axis: int) -> (FArray gradient)
routine FullGradient()          -> (Array<FArray> gradients)

// Equality — compared against any FArray
routine Equal   (other: FArray) -> (bool equal)
routine NotEqual(other: FArray) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (FArray farray)
```

---

## Example

```fal
import "libs/falconCore/math/arrays/measuredArray/measuredArray.fal";
import "libs/falconCore/generic/farray/farray.fal";

farray::FArray fa = farray::FArray.FromJSON(json);
MeasuredArray  ma = MeasuredArray.New(fa);

MeasuredArray scaled  = ma.Times(2.0);
MeasuredArray shifted = ma.Add(0.5);

float lo = ma.Min();
float hi = ma.Max();

bool same = ma.Equal(fa);

string json2 = ma.ToJSON();
```
