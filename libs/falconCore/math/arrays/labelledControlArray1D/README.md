# falcon/falconCore/math/arrays/labelledControlArray1D

Falcon binding for `falcon_core::math::arrays::LabelledControlArray1D` — a 1D control array tagged with an `AcquisitionContext` label (instrument port, connection, units).

---

## Installation

```fal
import "libs/falconCore/math/arrays/labelledControlArray1D/labelledControlArray1D.fal";
```

---

## Overview

`LabelledControlArray1D` extends `LabelledControlArray` with `Is1D<double>` semantics.  On construction it wraps an existing `FArray<double>` and attaches a default instrument-port label.  All arithmetic operations propagate the label and return a new `LabelledControlArray1D`.  Additional 1D accessors expose start/end values, monotonicity checks, statistics, and subdivision helpers.

---

## API

```fal
// Constructor — wrap an existing FArray with a default port label
routine New(arr: FArray) -> (LabelledControlArray1D carray)

// Arithmetic (element-wise; overloaded for LabelledControlArray1D, FArray, int, float)
routine Add    (other)     -> (LabelledControlArray1D carray)
routine Minus  (other)     -> (LabelledControlArray1D carray)
routine Negate ()          -> (LabelledControlArray1D carray)
routine Times  (factor)    -> (LabelledControlArray1D carray)
routine Divides(factor)    -> (LabelledControlArray1D carray)
routine Power  (exponent)  -> (LabelledControlArray1D carray)
routine Abs    ()          -> (LabelledControlArray1D carray)

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
routine Reshape(shape: Array<int>) -> (LabelledControlArray1D carray)
routine Flip   (axis: int)         -> (LabelledControlArray1D carray)

// Gradient
routine Gradient    (axis: int) -> (FArray gradient)
routine FullGradient()          -> (Array<FArray> gradients)

// Equality
routine Equal   (other: LabelledControlArray1D) -> (bool equal)
routine NotEqual(other: LabelledControlArray1D) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (LabelledControlArray1D carray)
```

---

## Example

```fal
import "libs/falconCore/math/arrays/labelledControlArray1D/labelledControlArray1D.fal";
import "libs/falconCore/generic/farray/farray.fal";

// Construct from an existing FArray
farray::FArray fa = farray::FArray.FromJSON(json);
labelledControlArray1D::LabelledControlArray1D lca1d = labelledControlArray1D::LabelledControlArray1D.New(fa);

// 1D accessors
float start = lca1d.GetStart();
float end   = lca1d.GetEnd();
bool  inc   = lca1d.IsIncreasing();
float mean  = lca1d.GetMean();
int   idx   = lca1d.GetClosestIndex(1.5);

// Arithmetic
labelledControlArray1D::LabelledControlArray1D scaled = lca1d.Times(2.0);

// Control metadata
int dim = lca1d.PrincipleDimension();
increasingAlignment::IncreasingAlignment al = lca1d.Alignment();

// Serialisation round-trip
string json2 = lca1d.ToJSON();
labelledControlArray1D::LabelledControlArray1D rt = labelledControlArray1D::LabelledControlArray1D.FromJSON(json2);
bool same = lca1d.Equal(rt);
```
