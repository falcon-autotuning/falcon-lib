# falcon/falconCore/math/discreteSpaces/discretizer

Falcon binding for `falcon_core::math::discrete_spaces::Discretizer` — a step-size discretizer for Cartesian or polar coordinate spaces, backed by a delta `Domain`.

---

## Installation

```fal
import "libs/falconCore/math/discreteSpaces/discretizer/discretizer.fal";
```

---

## Overview

A `Discretizer` encapsulates a step size (`delta`) and its valid `Domain`, along with the coordinate type (Cartesian or Polar).  Use `NewCartesianDiscretizer` for square grids and `NewPolarDiscretizer` for angular coordinates.

---

## API

```fal
// Constructors (static)
routine NewCartesianDiscretizer(float delta) -> (Discretizer disc)
routine NewPolarDiscretizer    (float delta) -> (Discretizer disc)

// Accessors
routine Delta   ()              -> (float d)
routine SetDelta(float delta)   -> ()
routine Domain  ()              -> (Domain d)

// Predicates
routine IsCartesian() -> (bool IsCartesian)
routine IsPolar    () -> (bool IsPolar)

// Equality
routine Equal   (Discretizer other) -> (bool equal)
routine NotEqual(Discretizer other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (Discretizer dstate)
```

---

## Example

```fal
import "libs/falconCore/math/discreteSpaces/discretizer/discretizer.fal";

Discretizer cart = Discretizer.NewCartesianDiscretizer(0.1);
Discretizer pol  = Discretizer.NewPolarDiscretizer(0.05);

float  d     = cart.Delta();          // 0.1
bool   isC   = cart.IsCartesian();    // true
bool   isP   = pol.IsPolar();         // true

Domain dom   = cart.Domain();         // delta domain for step size

cart.SetDelta(0.2);
float  d2    = cart.Delta();          // 0.2

bool   eq    = cart.Equal(cart);      // true
bool   ne    = cart.NotEqual(pol);    // true

string json  = cart.ToJSON();
Discretizer rt = Discretizer.FromJSON(json);
bool   same  = cart.Equal(rt);        // true
```
