# falcon/falconCore/math/domains/domain

Falcon binding for `falcon_core::math::domains::Domain` — a closed or half-open real-valued interval with rich arithmetic operations.

---

## Installation

```fal
import "libs/falconCore/math/domains/domain/domain.fal";
```

---

## Overview

A `Domain` represents a bounded interval `[min, max]` (or open/half-open variants) on the real line.  It supports containment tests, set-theoretic operations (intersection, union), affine transformations, and JSON serialisation.

---

## API

```fal
// Constructor
routine New(float min, float max,
            bool lesser_bound_contained, bool greater_bound_contained)
    -> (Domain d)

// Bound accessors
routine LesserBound()           -> (float min)
routine GreaterBound()          -> (float max)
routine LesserBoundContained()  -> (bool contained)
routine GreaterBoundContained() -> (bool contained)

// Queries
routine In(float value)         -> (bool contains)
routine Range()                 -> (float range)
routine GetCenter()             -> (float center)
routine IsEmpty()               -> (bool empty)
routine ContainsDomain(Domain other) -> (bool contains)

// Set operations (return new Domain)
routine Intersection(Domain other)         -> (Domain result)
routine Union(Domain other)                -> (Domain result)
routine Shift(float offset)                -> (Domain shifted)
routine Scaled(float offset)               -> (Domain scaled)

// Transform a value from this domain into another domain
routine Transform(Domain other, float value) -> (float scale)

// Equality
routine Equal   (Domain other) -> (bool equal)
routine NotEqual(Domain other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (Domain dstate)
```

---

## Example

```fal
import "libs/falconCore/math/domains/domain/domain.fal";

Domain d1 = Domain.New(0.0, 1.0, true, true);
Domain d2 = Domain.New(0.5, 1.5, true, true);

float  lb   = d1.LesserBound();    // 0.0
float  gb   = d1.GreaterBound();   // 1.0
bool   has  = d1.In(0.5);          // true
float  rng  = d1.Range();          // 1.0
float  ctr  = d1.GetCenter();      // 0.5

Domain inter = d1.Intersection(d2);   // [0.5, 1.0]
Domain union_ = d1.Union(d2);          // [0.0, 1.5]
Domain shifted = d1.Shift(1.0);        // [1.0, 2.0]
Domain scaled  = d1.Scaled(2.0);       // [0.0, 2.0]

float t = d1.Transform(d2, 0.0);      // mapped value

bool eq = d1.Equal(d1);               // true
bool ne = d1.NotEqual(d2);            // true

string json = d1.ToJSON();
Domain rt   = Domain.FromJSON(json);
bool   same = d1.Equal(rt);           // true
```
