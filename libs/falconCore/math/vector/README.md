# falcon/falconCore/math/vector

Falcon binding for `falcon_core::math::Vector` — a directed vector in multi-dimensional voltage space defined by start and end `Point` objects.

---

## Installation

```fal
import "libs/falconCore/math/vector/vector.fal";
```

---

## Overview

A `Vector` encodes displacement between two `Point` objects, shares a common `SymbolUnit`, and supports arithmetic, geometric transformations, projection, and JSON serialisation.

---

## API

```fal
// Constructors
routine NewFromStart(Point start)              -> (Vector v)  // end = start, start = origin
routine New        (Point start, Point end)   -> (Vector v)

// Accessors
routine EndPoint          ()               -> (Point p)
routine StartPoint        ()               -> (Point p)
routine Connections       ()               -> (Array<Connection> conns)
routine Unit              ()               -> (SymbolUnit unit)
routine PrincipleConnection()              -> (Connection conn)
routine Magnitude         ()               -> (float m)

// Arithmetic
routine Times   (float factor)  -> (Vector scaled_state)
routine Times   (int   factor)  -> (Vector scaled_state)
routine Divides (float divisor) -> (Vector scaled_state)
routine Divides (int   divisor) -> (Vector scaled_state)
routine Add     (Vector other)  -> (Vector sum_state)
routine Subtract(Vector other)  -> (Vector diff)
routine Negate                  -> (Vector negated_state)

// Geometric transformations
routine UpdateStartFromStates(DeviceVoltageStates states) -> (Vector v)
routine Translate            (Point point)                -> (Vector v)
routine TranslateToOrigin    ()                           -> (Vector v)
routine Extend               (float extension)            -> (Vector v)
routine Extend               (int   extension)            -> (Vector v)
routine Shrink               (float extension)            -> (Vector v)
routine Shrink               (int   extension)            -> (Vector v)
routine UnitVector           ()                           -> (Vector v)
routine Normalize            ()                           -> (Vector v)
routine Project              (Vector other)               -> (Vector v)
routine UpdateUnit           (SymbolUnit unit)            -> ()

// Equality
routine Equal   (Vector other) -> (bool equal)
routine NotEqual(Vector other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (Vector dstate)
```

---

## Example

```fal
import "libs/falconCore/math/vector/vector.fal";
import "libs/falconCore/math/point/point.fal";

Point  start = Point.NewFromQuantity(Map.New());
Point  end   = Point.NewFromQuantity(Map.New());
Vector v     = Vector.New(start, end);

float  mag  = v.Magnitude();        // 0.0 for empty points
Vector neg  = v.Negate();
bool   eq   = v.Equal(v);           // true

string json = v.ToJSON();
Vector rt   = Vector.FromJSON(json);
bool   same = v.Equal(rt);          // true
```
