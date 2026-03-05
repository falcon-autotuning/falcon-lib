# falcon/falconCore/physics/deviceStructures/connection

Falcon binding for `falconCore::physics::device_structures::Connection` — a named, typed quantum-device terminal (gate or ohmic contact).

---

## Installation

```fal
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
```

---

## Overview

A `Connection` represents a single electrical contact on a quantum device.  Each connection has a **name** (e.g. `"P1"`, `"B2"`) and a **type** drawn from one of five device-feature categories:

| Type | Factory |
|------|---------|
| Plunger gate | `Connection.NewPlungerGate(name)` |
| Barrier gate | `Connection.NewBarrierGate(name)` |
| Reservoir gate | `Connection.NewReservoirGate(name)` |
| Screening gate | `Connection.NewScreeningGate(name)` |
| Ohmic contact | `Connection.NewOhmic(name)` |

---

## API

```fal
// Constructors
routine NewPlungerGate  (string name) -> (Connection c)
routine NewBarrierGate  (string name) -> (Connection c)
routine NewReservoirGate(string name) -> (Connection c)
routine NewScreeningGate(string name) -> (Connection c)
routine NewOhmic        (string name) -> (Connection c)

// Accessors
routine Name            -> (string name)
routine Type            -> (string type)

// Type predicates
routine IsDotGate       -> (bool is)   // true for plunger or barrier
routine IsBarrierGate   -> (bool is)
routine IsPlungerGate   -> (bool is)
routine IsScreeningGate -> (bool is)
routine IsReservoirGate -> (bool is)
routine IsOhmic         -> (bool is)
routine IsGate          -> (bool is)   // true for any non-ohmic

// Equality
routine Equal   (Connection other) -> (bool equal)
routine NotEqual(Connection other) -> (bool notequal)

// Serialisation
routine ToJSON  ()           -> (string json)
routine FromJSON(string json) -> (Connection c)
```

---

## Example

```fal
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";

Connection p1 = Connection.NewPlungerGate("P1");
Connection b1 = Connection.NewBarrierGate("B1");
Connection s1 = Connection.NewOhmic("S1");

string name = p1.Name();          // "P1"
string type = p1.Type();          // "PlungerGate"
bool   isG  = p1.IsGate();        // true
bool   isO  = s1.IsOhmic();       // true
bool   same = p1.Equal(p1);       // true
bool   diff = p1.NotEqual(b1);    // true
```
