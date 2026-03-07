# falcon/falconCore/math/quantity

Falcon binding for `falcon_core::math::Quantity` — a physical value paired with a `SymbolUnit`, supporting arithmetic, unit conversion, and JSON serialisation.

---

## Installation

```fal
import "libs/falconCore/math/quantity/quantity.fal";
```

---

## Overview

A `Quantity` wraps a `double` value and a `SymbolUnit`.  It supports the full suite of arithmetic operators (`+`, `-`, `*`, `/`, `^`), unary negation, absolute value, in-place unit conversion, and equality comparison.

---

## API

```fal
// Constructor
routine New(float value, SymbolUnit unit) -> (Quantity q)

// Accessors
routine Value()               -> (float out)
routine Unit()                -> (SymbolUnit unit)

// Unit conversion (returns a converted copy)
routine ConvertToUnit(SymbolUnit new_unit) -> (Quantity converted_state)

// Arithmetic — scalar overloads
routine Times   (float factor)            -> (Quantity scaled_state)
routine Times   (int   factor)            -> (Quantity scaled_state)
routine Times   (Quantity factor)         -> (Quantity scaled_state)
routine Divides (float divisor)           -> (Quantity scaled_state)
routine Divides (int   divisor)           -> (Quantity scaled_state)
routine Divides (Quantity divisor)        -> (Quantity scaled_state)
routine Power   (float exponent)          -> (Quantity powered_state)
routine Add     (Quantity other)          -> (Quantity sum_state)
routine Subtract(Quantity other)          -> (Quantity difference_state)
routine Negate                            -> (Quantity negated_state)
routine Abs                               -> (Quantity absolute_state)

// Equality
routine Equal   (Quantity other) -> (bool equal)
routine NotEqual(Quantity other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (Quantity dstate)
```

---

## Example

```fal
import "libs/falconCore/math/quantity/quantity.fal";
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";

SymbolUnit volt = SymbolUnit.Volt();
Quantity   q    = Quantity.New(2.0, volt);

float  val  = q.Value();              // 2.0
Quantity q2 = q.Times(3.0);           // 6.0 V
Quantity q3 = q.Add(Quantity.New(1.0, volt));  // 3.0 V
Quantity q4 = q.Negate();             // -2.0 V
Quantity q5 = q.Abs();                // 2.0 V
Quantity q6 = q.Power(2.0);           // 4.0 V²

bool eq = q.Equal(Quantity.New(2.0, volt));  // true
bool ne = q.NotEqual(q2);                    // true

string json = q.ToJSON();
Quantity rt  = Quantity.FromJSON(json);
bool same   = q.Equal(rt);            // true
```
