# falcon/falconCore/math/arrays/controlArray1D

Falcon binding for `falcon_core::math::arrays::ControlArray1D` — a one-dimensional control array with alignment tracking and 1D-specific accessors.

---

## Installation

```fal
import "libs/falconCore/math/arrays/controlArray1D/controlArray1D.fal";
```

---

## Overview

`ControlArray1D` extends `ControlArray` (and `Is1D<double>`) to represent a strictly one-dimensional axis vector.  In addition to the full `ControlArray` arithmetic, it exposes start/end accessors, statistical reductions, a `Reverse` mutation, nearest-index lookup, and an even-division split.

---

## API

```fal
// Constructor
routine New(arr: FArray) -> (ControlArray1D carray)

// Arithmetic — same overloads as ControlArray; Add also accepts ControlArray
routine Add    (other)    -> (ControlArray1D carray)
routine Minus  (other)    -> (ControlArray1D carray)
routine Negate ()         -> (ControlArray1D carray)
routine Times  (factor)   -> (ControlArray1D carray)
routine Divides(factor)   -> (ControlArray1D carray)
routine Power  (exponent) -> (ControlArray1D carray)
routine Abs    ()         -> (ControlArray1D carray)

// Reduction
routine Min() -> (float min)
routine Max() -> (float max)

// Control metadata
routine PrincipleDimension() -> (int dim)
routine Alignment()          -> (IncreasingAlignment alignment)

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
routine Reshape(shape: Array<int>) -> (ControlArray1D carray)
routine Flip   (axis: int)         -> (ControlArray1D carray)

// Gradient
routine Gradient    (axis: int) -> (FArray gradient)
routine FullGradient()          -> (Array<FArray> gradients)

// Equality
routine Equal   (other: ControlArray1D) -> (bool equal)
routine NotEqual(other: ControlArray1D) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (ControlArray1D carray)
```

---

## Example

```fal
import "libs/falconCore/math/arrays/controlArray1D/controlArray1D.fal";
import "libs/falconCore/generic/farray/farray.fal";

farray::FArray   fa  = farray::FArray.FromJSON(json);
ControlArray1D   ca  = ControlArray1D.New(fa);

float start = ca.GetStart();
float end   = ca.GetEnd();
bool  inc   = ca.IsIncreasing();
float dist  = ca.GetDistance();
float mean  = ca.GetMean();
int   idx   = ca.GetClosestIndex(1.5);

string json2      = ca.ToJSON();
ControlArray1D rt = ControlArray1D.FromJSON(json2);
bool same         = ca.Equal(rt);
```
