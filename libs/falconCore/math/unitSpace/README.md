# falcon/falconCore/math/unitSpace

Falcon binding for `falcon_core::math::UnitSpace` — a hypercube measurement space defined by discretised axes and a bounding `Domain`.

---

## Installation

```fal
import "libs/falconCore/math/unitSpace/unitSpace.fal";
```

---

## Overview

A `UnitSpace` combines a set of `Discretizer`-based axes with a `Domain` to represent a structured grid.  The most common entry points are `Cartesian1DSpace` and `RaySpace`.  Call `Compile` before accessing `Space` or `Shape`.

---

## API

```fal
// Constructors
routine New             (Axes<Discretizer> axes, Domain domain) -> (UnitSpace space)
routine RaySpace        (float dr, float dtheta, Domain domain) -> (UnitSpace space)
routine CartesianSpace  (Axes<float> deltas, Domain domain)     -> (UnitSpace space)
routine Cartesian1DSpace(float delta, Domain domain)            -> (UnitSpace space)
routine Cartesian2DSpace(Axes<float> deltas, Domain domain)     -> (UnitSpace space)

// Accessors
routine Axes      () -> (Axes<Discretizer> axes)
routine Domain    () -> (Domain domain)
routine Space     () -> (FArray space)
routine Shape     () -> (Array<int> shape)
routine Dimension () -> (int size)

// Operations
routine Compile     ()                     -> ()
routine CreateArray (Axes<int> axes)       -> (Axes<ControlArray> arrays)

// Equality
routine Equal   (UnitSpace other) -> (bool equal)
routine NotEqual(UnitSpace other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (UnitSpace space)
```

> **Note:** `New`, `CartesianSpace`, and `Cartesian2DSpace` accept an `Axes` parameter that is complex to construct; they currently fall back to a 1D Cartesian space with δ = 0.1 using the supplied domain.  `CreateArray` returns NIL until the full `ControlArray` binding is available.

---

## Example

```fal
import "libs/falconCore/math/unitSpace/unitSpace.fal";
import "libs/falconCore/math/domains/domain/domain.fal";

Domain    d  = Domain.New(0.0, 1.0, true, true);
UnitSpace us = UnitSpace.Cartesian1DSpace(0.1, d);

int  dim  = us.Dimension();   // 1
us.Compile();

bool eq   = us.Equal(us);     // true

string json = us.ToJSON();
UnitSpace rt = UnitSpace.FromJSON(json);
bool same = us.Equal(rt);     // true
```
