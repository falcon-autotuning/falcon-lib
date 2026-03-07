# falcon/falconCore/math/arrays/labelledMeasuredArray

Falcon binding for `falcon_core::math::arrays::LabelledMeasuredArray` — a measured array tagged with an `AcquisitionContext` label (instrument port, connection, units).

---

## Installation

```fal
import "libs/falconCore/math/arrays/labelledMeasuredArray/labelledMeasuredArray.fal";
```

---

## Overview

`LabelledMeasuredArray` extends `MeasuredArray` with a label derived from an `AcquisitionContext`.  On construction it wraps an existing `FArray<double>` and attaches a default instrument-port label.  All arithmetic operations propagate the label and return a new `LabelledMeasuredArray`.  Equality is tested against plain `FArray` values (comparing the underlying data).

---

## API

```fal
// Constructor — wrap an existing FArray with a default port label
routine New(arr: FArray) -> (LabelledMeasuredArray carray)

// Arithmetic (element-wise; overloaded for MeasuredArray, FArray, int, float)
routine Add    (other)     -> (LabelledMeasuredArray carray)
routine Minus  (other)     -> (LabelledMeasuredArray carray)
routine Negate ()          -> (LabelledMeasuredArray carray)
routine Times  (factor)    -> (LabelledMeasuredArray carray)
routine Divides(factor)    -> (LabelledMeasuredArray carray)
routine Power  (exponent)  -> (LabelledMeasuredArray carray)
routine Abs    ()          -> (LabelledMeasuredArray carray)

// Reduction
routine Min() -> (float min)
routine Max() -> (float max)

// Label metadata
routine Label         () -> (AcquisitionContext ac)
routine Connection    () -> (Connection conn)
routine InstrumentType() -> (string it)
routine Units         () -> (SymbolUnit unit)

// Shape
routine Reshape(shape: Array<int>) -> (LabelledMeasuredArray carray)
routine Flip   (axis: int)         -> (LabelledMeasuredArray carray)

// Gradient
routine Gradient    (axis: int) -> (FArray gradient)
routine FullGradient()          -> (Array<FArray> gradients)

// Equality (compared against the underlying FArray data)
routine Equal   (other: FArray) -> (bool equal)
routine NotEqual(other: FArray) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (FArray farray)
```

---

## Example

```fal
import "libs/falconCore/math/arrays/labelledMeasuredArray/labelledMeasuredArray.fal";
import "libs/falconCore/generic/farray/farray.fal";

// Construct from an existing FArray
farray::FArray fa = farray::FArray.FromJSON(json);
labelledMeasuredArray::LabelledMeasuredArray lma = labelledMeasuredArray::LabelledMeasuredArray.New(fa);

// Arithmetic
labelledMeasuredArray::LabelledMeasuredArray scaled  = lma.Times(2.0);
labelledMeasuredArray::LabelledMeasuredArray shifted = lma.Add(1.0);
labelledMeasuredArray::LabelledMeasuredArray power   = lma.Power(2.0);

// Label metadata
acquisitionContext::AcquisitionContext ac = lma.Label();

// Equality against source FArray
bool same = lma.Equal(fa);

// Serialisation
string json2      = lma.ToJSON();
farray::FArray rt = labelledMeasuredArray::LabelledMeasuredArray.FromJSON(json2);
```
