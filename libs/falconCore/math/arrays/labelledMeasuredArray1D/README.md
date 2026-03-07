# falcon/falconCore/math/arrays/labelledMeasuredArray1D

Falcon binding for `falcon_core::math::arrays::LabelledMeasuredArray1D` — a 1D measured array tagged with an `AcquisitionContext` label (instrument port, connection, units).

---

## Installation

```fal
import "libs/falconCore/math/arrays/labelledMeasuredArray1D/labelledMeasuredArray1D.fal";
```

---

## Overview

`LabelledMeasuredArray1D` extends `LabelledMeasuredArray` with `Is1D<double>` semantics.  On construction it wraps an existing `FArray<double>` and attaches a default instrument-port label.  All arithmetic operations propagate the label and return a new `LabelledMeasuredArray1D`.  Additional 1D accessors expose start/end values, monotonicity checks, statistics, and subdivision helpers.

---

## API

```fal
// Constructor — wrap an existing FArray with a default port label
routine New(arr: FArray) -> (LabelledMeasuredArray1D carray)

// Arithmetic (element-wise; overloaded for LabelledMeasuredArray1D, MeasuredArray, FArray, int, float)
routine Add    (other)     -> (LabelledMeasuredArray1D carray)
routine Minus  (other)     -> (LabelledMeasuredArray1D carray)
routine Negate ()          -> (LabelledMeasuredArray1D carray)
routine Times  (factor)    -> (LabelledMeasuredArray1D carray)
routine Divides(factor)    -> (LabelledMeasuredArray1D carray)
routine Power  (exponent)  -> (LabelledMeasuredArray1D carray)
routine Abs    ()          -> (LabelledMeasuredArray1D carray)

// Reduction
routine Min() -> (float min)
routine Max() -> (float max)

// Label metadata
routine Label         () -> (AcquisitionContext ac)
routine Connection    () -> (Connection conn)
routine InstrumentType() -> (string it)
routine Units         () -> (SymbolUnit unit)

// 1D accessors
routine GetStart        ()               -> (float start)
routine GetEnd          ()               -> (float end)
routine IsDecreasing    ()               -> (bool decreasing)
routine IsIncreasing    ()               -> (bool increasing)
routine GetDistance     ()               -> (float distance)
routine GetMean         ()               -> (float mean)
routine GetSTD          ()               -> (float std)
routine Reverse         ()               -> ()
routine GetClosestIndex (value: float)   -> (int index)
routine EvenDivisions   (divisions: int) -> (Array<FArray> splits)

// Shape
routine Reshape(shape: Array<int>) -> (LabelledMeasuredArray1D carray)
routine Flip   (axis: int)         -> (LabelledMeasuredArray1D carray)

// Gradient
routine Gradient    (axis: int) -> (FArray gradient)
routine FullGradient()          -> (Array<FArray> gradients)

// Equality
routine Equal   (other: LabelledMeasuredArray1D) -> (bool equal)
routine NotEqual(other: LabelledMeasuredArray1D) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (LabelledMeasuredArray1D farray)
```

---

## Example

```fal
import "libs/falconCore/math/arrays/labelledMeasuredArray1D/labelledMeasuredArray1D.fal";
import "libs/falconCore/generic/farray/farray.fal";

// Construct from an existing FArray
farray::FArray fa = farray::FArray.FromJSON(json);
labelledMeasuredArray1D::LabelledMeasuredArray1D lma1d = labelledMeasuredArray1D::LabelledMeasuredArray1D.New(fa);

// 1D accessors
float start = lma1d.GetStart();
float end   = lma1d.GetEnd();
bool  inc   = lma1d.IsIncreasing();
float mean  = lma1d.GetMean();
int   idx   = lma1d.GetClosestIndex(1.5);

// Arithmetic
labelledMeasuredArray1D::LabelledMeasuredArray1D scaled = lma1d.Times(2.0);

// Label metadata
acquisitionContext::AcquisitionContext ac = lma1d.Label();

// Serialisation round-trip
string json2 = lma1d.ToJSON();
labelledMeasuredArray1D::LabelledMeasuredArray1D rt = labelledMeasuredArray1D::LabelledMeasuredArray1D.FromJSON(json2);
bool same = lma1d.Equal(rt);
```
