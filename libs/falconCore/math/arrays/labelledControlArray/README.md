# falcon/falconCore/math/arrays/labelledControlArray

Falcon binding for `falcon_core::math::arrays::LabelledControlArray` — a control array tagged with an `AcquisitionContext` label (instrument port, connection, units).

---

## Installation

```fal
import "libs/falconCore/math/arrays/labelledControlArray/labelledControlArray.fal";
```

---

## Overview

`LabelledControlArray` extends `ControlArray` with a label derived from an `AcquisitionContext`.  On construction it wraps an existing `FArray<double>` and attaches a default instrument-port label.  All arithmetic operations propagate the label and return a new `LabelledControlArray`.

---

## API

```fal
// Constructor — wrap an existing FArray with a default port label
routine New(arr: FArray) -> (LabelledControlArray carray)

// Arithmetic (element-wise; overloaded for LabelledControlArray, FArray, int, float)
routine Add    (other)     -> (LabelledControlArray carray)
routine Minus  (other)     -> (LabelledControlArray carray)
routine Negate ()          -> (LabelledControlArray carray)
routine Times  (factor)    -> (LabelledControlArray carray)
routine Divides(factor)    -> (LabelledControlArray carray)
routine Power  (exponent)  -> (LabelledControlArray carray)
routine Abs    ()          -> (LabelledControlArray carray)

// Reduction
routine Min() -> (float min)
routine Max() -> (float max)

// Control metadata
routine PrincipleDimension() -> (int dim)
routine Alignment()          -> (IncreasingAlignment alignment)

// Label metadata
routine Label         () -> (AcquisitionContext ac)
routine Connection    () -> (Connection conn)
routine InstrumentType() -> (string it)
routine Units         () -> (SymbolUnit unit)

// Shape
routine Reshape(shape: Array<int>) -> (LabelledControlArray carray)
routine Flip   (axis: int)         -> (LabelledControlArray carray)

// Gradient
routine Gradient    (axis: int) -> (FArray gradient)
routine FullGradient()          -> (Array<FArray> gradients)

// Equality
routine Equal   (other: LabelledControlArray) -> (bool equal)
routine NotEqual(other: LabelledControlArray) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (LabelledControlArray carray)
```

---

## Example

```fal
import "libs/falconCore/math/arrays/labelledControlArray/labelledControlArray.fal";
import "libs/falconCore/generic/farray/farray.fal";

// Construct from an existing FArray
farray::FArray fa = farray::FArray.FromJSON(json);
labelledControlArray::LabelledControlArray lca = labelledControlArray::LabelledControlArray.New(fa);

// Arithmetic
labelledControlArray::LabelledControlArray scaled  = lca.Times(2.0);
labelledControlArray::LabelledControlArray shifted = lca.Add(1.0);
labelledControlArray::LabelledControlArray power   = lca.Power(2.0);

// Control metadata
int dim = lca.PrincipleDimension();
increasingAlignment::IncreasingAlignment al = lca.Alignment();

// Label metadata
acquisitionContext::AcquisitionContext ac = lca.Label();

// Serialisation round-trip
string json2                               = lca.ToJSON();
labelledControlArray::LabelledControlArray rt = labelledControlArray::LabelledControlArray.FromJSON(json2);
bool same = lca.Equal(rt);
```
