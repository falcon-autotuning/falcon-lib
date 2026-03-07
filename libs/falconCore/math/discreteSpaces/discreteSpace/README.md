# falcon/falconCore/math/discreteSpaces/discreteSpace

Falcon binding for `falcon_core::math::discrete_spaces::DiscreteSpace` — a cartesian or polar discrete measurement space backed by a `UnitSpace` and coupled labelled domain axes.

---

## Installation

```fal
import "libs/falconCore/math/discreteSpaces/discreteSpace/discreteSpace.fal";
```

---

## Overview

> **Status:** work-in-progress.  The public FAL interface currently exposes only equality and JSON serialisation; construction helpers are available in the test-only binding `tests/discrete-space-test-helpers.fal`.

A `DiscreteSpace` pairs a compiled `UnitSpace` with per-axis `CoupledLabelledDomain` objects and monotonicity flags.

---

## API

```fal
// Equality
routine Equal   (DiscreteSpace other) -> (bool equal)
routine NotEqual(DiscreteSpace other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (DiscreteSpace dstate)
```

---

## Example

```fal
import "libs/falconCore/math/discreteSpaces/discreteSpace/discreteSpace.fal";

// Construct via JSON round-trip (no public FAL constructor yet).
// Use the test helper for unit tests:
//   import "tests/discrete-space-test-helpers.fal";
//   DiscreteSpace ds = DiscreteSpaceTestHelpers.New1D();

string json = ds.ToJSON();
DiscreteSpace rt = DiscreteSpace.FromJSON(json);
bool same = ds.Equal(rt);   // true
```
