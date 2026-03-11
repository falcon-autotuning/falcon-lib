# falcon/falconCore/math/discreteSpaces/discreteSpace

Falcon FFI binding for `falcon_core::math::discrete_spaces::DiscreteSpace` — a
Cartesian (or general ND) discrete measurement space backed by a compiled
`UnitSpace` and per-axis `CoupledLabelledDomain` objects with monotonicity flags.

---

## Installation

```fal
import "libs/falconCore/math/discreteSpaces/discreteSpace/discreteSpace.fal";
```

---

## Overview

A `DiscreteSpace` associates a compiled `UnitSpace` with a set of labelled,
coupled domain axes (`CoupledLabelledDomain`) and a per-axis map that records
whether each axis increases or decreases in the direction of the domain.

The most common entry-point is `CartesianDiscreteSpace1D`, which builds a
single-axis Cartesian space from a division count, a coupled domain, a
monotonicity map, and a bounding `Domain`.

---

## Example

```fal
import "libs/falconCore/math/discreteSpaces/discreteSpace/discreteSpace.fal";
import "libs/falconCore/math/domains/coupledLabelledDomain/coupledLabelledDomain.fal";
import "libs/falconCore/math/domains/domain/domain.fal";
import "libs/falconCore/collections/map/map.fal";
import "libs/falconCore/instrumentInterfaces/names/instrumentPort/instrumentPort.fal";

// Build a 1-D Cartesian space with 10 divisions over [0, 1].
Domain                dom = Domain.New(0.0, 1.0);
InstrumentPort        ip  = InstrumentPort.New("freq");
LabelledDomain        ld  = LabelledDomain.New(ip, dom);
CoupledLabelledDomain cld = CoupledLabelledDomain.New([ld]);
Map<string, bool>     inc = Map.New();          // empty → default monotonicity

DiscreteSpace ds = DiscreteSpace.CartesianDiscreteSpace1D(10, cld, inc, dom);

// Introspection
UnitSpace             us   = ds.Space();        // underlying compiled space
Ports                 knobs = ds.Knobs();       // ["freq"]
int                   axis  = ds.GetAxis(ip);   // 0
Domain                d     = ds.GetDomain(ip); // [0, 1]

// Projection
Axes<LabelledControlArray> proj = ds.GetProjection([ip]);

// Serialisation round-trip
string        json = ds.ToJSON();
DiscreteSpace rt   = DiscreteSpace.FromJSON(json);
bool          same = ds.Equal(rt);              // true
```

---

## Testing

```sh
falcon test libs/falconCore/math/discreteSpaces/discreteSpace/
```

The test suite (`tests/run_tests.fal`) covers:

| Suite | What is tested |
|---|---|
| `DiscreteSpaceConstruction` | `CartesianDiscreteSpace1D` produces a valid, non-empty-JSON object |
| `DiscreteSpaceAccessorSpace` | `Space()` returns a non-empty `UnitSpace` |
| `DiscreteSpaceAccessorIncreasing` | `Increasing()` returns an `Axes` of the correct length |
| `DiscreteSpaceAccessorKnobs` | `Knobs()` returns a `Ports` with non-negative size |
| `DiscreteSpaceGetAxis` | `GetAxis()` returns the correct zero-based index |
| `DiscreteSpaceGetDomain` | `GetDomain()` returns a non-empty `Domain` |
| `DiscreteSpaceGetProjection` | `GetProjection()` returns the correct number of arrays |
| `DiscreteSpaceEquality` | `Equal`/`NotEqual` — self, round-trip |
| `DiscreteSpaceJSON` | `ToJSON`/`FromJSON` — non-empty, round-trip equality, JSON stability |
