# falcon/falconCore/physics/deviceStructures/connections

Falcon binding for `falcon_core::physics::device_structures::Connections` — an ordered, serialisable collection of `Connection` objects with type-predicate helpers and set-intersection support.

---

## Installation

```fal
import "libs/falconCore/physics/deviceStructures/connections/connections.fal";
```

---

## Dependencies

| Module | Path |
|--------|------|
| `connection` | `libs/falconCore/physics/deviceStructures/connection/connection.fal` |
| `array` | `libs/falconCore/collections/array/array.fal` |

---

## Overview

`Connections` is a `List<Connection>` that adds bulk type-predicate queries (`IsGates`, `IsOhmics`, etc.) and a set-intersection operation.  Construct it with `New(Array<Connection>)` or deserialise from JSON.

---

## API

```fal
// Constructor
routine New(Array<Connection> conns) -> (Connections c)

// Type predicates — true when every element satisfies the predicate
routine IsGates          -> (bool IsGates)
routine IsOhmics         -> (bool IsOhmics)
routine IsDotGates       -> (bool IsDotGates)
routine IsPlungerGates   -> (bool IsPlungerGates)
routine IsBarrierGates   -> (bool IsBarrierGates)
routine IsReservoirGates -> (bool IsReservoirGates)
routine IsScreeningGates -> (bool IsScreeningGates)

// Set operations
routine Intersection(Connections other) -> (Connections out)

// Equality
routine Equal   (Connections other) -> (bool equal)
routine NotEqual(Connections other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (Connections conn)

// List operations
routine Size     ()                    -> (int size)
routine IsEmpty  ()                    -> (bool empty)
routine GetIndex (int index)           -> (Connection value)
routine PushBack (Connection value)    -> ()
routine Insert   (int index, Connection value) -> ()
routine Contains (Connection value)    -> (bool found)
routine Index    (Connection value)    -> (int index)
routine Items    ()                    -> (Array<Connection> items)
routine Clear    ()                    -> ()
routine Erase    (int index)           -> ()
```

---

## Example

```fal
import "libs/falconCore/physics/deviceStructures/connections/connections.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";

Connection p1 = Connection.NewPlungerGate("P1");
Connection b1 = Connection.NewBarrierGate("B1");
Connection s1 = Connection.NewOhmic("S1");

Connections gates = Connections.New([p1, b1]);
bool allGates = gates.IsGates();    // true
bool allOhmics = gates.IsOhmics();  // false
int  sz       = gates.Size();       // 2

gates.PushBack(s1);
bool found = gates.Contains(s1);    // true

string json  = gates.ToJSON();
Connections restored = Connections.FromJSON(json);
bool same = gates.Equal(restored);  // true
```
