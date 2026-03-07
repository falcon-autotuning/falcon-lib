# falcon/falconCore/math/point

Falcon binding for `falcon_core::math::Point` — a multi-dimensional point in voltage space, indexed by device connections with an SI unit.

---

## Installation

```fal
import "libs/falconCore/math/point/point.fal";
```

---

## Overview

A `Point` maps device `Connection` keys to `Quantity` values, sharing a common `SymbolUnit`. It supports insertion, arithmetic, equality, and JSON serialisation.

---

## API

```fal
// Constructors
routine NewFromRaw     (Map<Connection, float>    init, SymbolUnit unit) -> (Point q)
routine NewFromQuantity(Map<Connection, Quantity> init)                  -> (Point q)

// Mutators
routine InsertOrAssign(Connection key, Quantity value) -> ()
routine Insert        (Connection key, Quantity value) -> ()
routine SetUnit       (SymbolUnit unit)                -> ()

// Accessors
routine Unit        () -> (SymbolUnit unit)
routine Coordinates () -> (Map<Connection, Quantity> map)
routine Connections () -> (Array<Connection> conns)

// Derived (implemented in FAL)
routine Size    () -> (int size)
routine IsEmpty () -> (bool empty)
routine Get     (Connection key)                    -> (float value)
routine GetOr   (Connection key, Quantity default_) -> (Quantity value)

// Arithmetic
routine Times   (float factor)  -> (Point scaled_state)
routine Divides (float divisor) -> (Point scaled_state)
routine Add     (Point other)   -> (Point sum_state)
routine Subtract(Point other)   -> (Point difference_state)
routine Negate                  -> (Point negated_state)

// Equality
routine Equal   (Point other) -> (bool equal)
routine NotEqual(Point other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (Point dstate)
```

---

## Example

```fal
import "libs/falconCore/math/point/point.fal";
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
import "libs/falconCore/math/quantity/quantity.fal";

SymbolUnit volt = SymbolUnit.Volt();
Point p = Point.NewFromRaw(Map.New(), volt);

Connection p1  = Connection.NewPlungerGate("P1");
Quantity   q1  = Quantity.New(0.5, volt);
p.InsertOrAssign(p1, q1);

int  sz  = p.Size();         // 1
bool emp = p.IsEmpty();      // false

Point p2  = p.Times(2.0);
Point neg = p.Negate();

bool eq = p.Equal(p);        // true

string json = p.ToJSON();
Point  rt   = Point.FromJSON(json);
bool   same = p.Equal(rt);   // true
```
