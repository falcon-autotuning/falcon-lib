# falcon/falconCore/generic/farray

Falcon binding for `falcon_core::generic::FArray` — an element-wise arithmetic array of floating-point quantities. Used as the fundamental multi-dimensional value type throughout the optimiser pipeline.

---

## Installation

```fal
import "libs/falconCore/generic/farray/farray.fal";
```

---

## Overview

`FArray` wraps a fixed-size vector of floats and exposes the complete arithmetic surface used by the optimiser: element-wise scaling, division, power, addition, subtraction, negation, absolute value, equality comparison, and JSON round-trip serialisation.

`FArray` has **no public constructor** in the `.fal` interface — instances are produced by other modules (e.g. `DeviceVoltageStates.ToPoint`). Tests use a test-helper shim to construct instances from float literals.

---

## API

```fal
// Equality
routine Equal   (FArray other) -> (bool equal)
routine NotEqual(FArray other) -> (bool notequal)

// Arithmetic — scale
routine Times  (float    factor)   -> (FArray scaled_state)
routine Times  (int      factor)   -> (FArray scaled_state)
routine Times  (Quantity factor)   -> (FArray scaled_state)
routine Divides(float    divisor)  -> (FArray scaled_state)
routine Divides(int      divisor)  -> (FArray scaled_state)
routine Divides(Quantity divisor)  -> (FArray scaled_state)
routine Power  (float    exponent) -> (FArray powered_state)

// Arithmetic — add / subtract
routine Add     (FArray other) -> (FArray sum_state)
routine Add     (int    other) -> (FArray sum_state)
routine Add     (float  other) -> (FArray sum_state)
routine Subtract(FArray other) -> (FArray difference_state)
routine Subtract(int    other) -> (FArray difference_state)
routine Subtract(float  other) -> (FArray difference_state)

// Unary
routine Negate -> (FArray negated_state)
routine Abs    -> (FArray absolute_state)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (FArray state)
```

---

## Example

```fal
// FArray instances are normally obtained from pipeline outputs, e.g.:
// FArray pt = myDeviceVoltageStates.ToPoint().Values();

// Round-trip serialisation:
string json = myFArray.ToJSON();
FArray copy = FArray.FromJSON(json);

// Arithmetic:
FArray scaled    = myFArray.Times(2.0);
FArray shifted   = myFArray.Add(0.1);
FArray reflected = myFArray.Negate();
FArray magnitude = myFArray.Abs();
```

---

## Testing

Tests live in `tests/run_tests.fal`. They use a private test-helper shim (`tests/farray-test-helpers.fal`) that binds `NewFromFloats` so tests can construct `FArray` values directly from float lists without exposing a public constructor.
