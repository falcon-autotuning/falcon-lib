# falcon/falconCore/math/arrays/measuredArray1D

Falcon binding for `falcon_core::math::arrays::MeasuredArray1D` — a one-dimensional measured array with 1D-specific accessors and arithmetic.

---

## Installation

```fal
import "libs/falconCore/math/arrays/measuredArray1D/measuredArray1D.fal";
```

---

## Overview

`MeasuredArray1D` extends `MeasuredArray` (and `Is1D<double>`) to represent a strictly one-dimensional sampled data vector.  It exposes the same element-wise arithmetic as `MeasuredArray` together with 1D-specific methods: start/end element access, statistical summaries, directional predicates, nearest-index lookup, and even-division splitting.

---

## API

```fal
// Constructor
routine New(arr: FArray) -> (MeasuredArray1D carray)

// Arithmetic (overloaded for MeasuredArray, FArray, int, float)
routine Add    (other)    -> (MeasuredArray1D carray)
routine Minus  (other)    -> (MeasuredArray1D carray)
routine Negate ()         -> (MeasuredArray1D carray)
routine Times  (factor)   -> (MeasuredArray1D carray)
routine Divides(factor)   -> (MeasuredArray1D carray)
routine Power  (exponent) -> (MeasuredArray1D carray)
routine Abs    ()         -> (MeasuredArray1D carray)

// Reduction
routine Min() -> (float min)
routine Max() -> (float max)

// 1D accessors
routine GetStart    ()               -> (float start)
routine GetEnd      ()               -> (float end)
routine GetDistance ()               -> (float distance)
routine GetMean     ()               -> (float mean)
routine GetSTD      ()               -> (float std)
routine IsIncreasing()               -> (bool increasing)
routine IsDecreasing()               -> (bool decreasing)
routine Reverse     ()               -> ()
routine GetClosestIndex(float value) -> (int index)
routine EvenDivisions  (int divs)    -> (Array<FArray> splits)

// Shape
routine Reshape(shape: Array<int>) -> (MeasuredArray1D carray)
routine Flip   (axis: int)         -> (MeasuredArray1D carray)

// Gradient
routine Gradient    (axis: int) -> (FArray gradient)
routine FullGradient()          -> (Array<FArray> gradients)

// Equality
routine Equal   (other: MeasuredArray1D) -> (bool equal)
routine NotEqual(other: MeasuredArray1D) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (MeasuredArray1D farray)
```

---

## Example

```fal
import "libs/falconCore/math/arrays/measuredArray1D/measuredArray1D.fal";
import "libs/falconCore/generic/farray/farray.fal";

farray::FArray    fa  = farray::FArray.FromJSON(json);
MeasuredArray1D   ma  = MeasuredArray1D.New(fa);

float start = ma.GetStart();
float end   = ma.GetEnd();
bool  inc   = ma.IsIncreasing();
float mean  = ma.GetMean();
int   idx   = ma.GetClosestIndex(1.5);

string json2       = ma.ToJSON();
MeasuredArray1D rt = MeasuredArray1D.FromJSON(json2);
bool same          = ma.Equal(rt);
```
