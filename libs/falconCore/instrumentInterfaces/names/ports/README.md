# falcon/falconCore/instrumentInterfaces/names/ports

Falcon binding for `falcon_core::instrument_interfaces::names::Ports` — a collection of `InstrumentPort` entries with bulk accessor helpers.

---

## Installation

```fal
import "libs/falconCore/instrumentInterfaces/names/ports/ports.fal";
```

---

## Dependencies

| Module | Path |
|--------|------|
| `connection` | `libs/falconCore/physics/deviceStructures/connection/connection.fal` |
| `symbolUnit` | `libs/falconCore/physics/units/symbolUnit/symbolUnit.fal` |
| `instrument` | `libs/falconCore/instrumentInterfaces/names/instrument/instrument.fal` |
| `instrumentPort` | `libs/falconCore/instrumentInterfaces/names/instrumentPort/instrumentPort.fal` |

---

## Overview

`Ports` is the top-level collection returned by the instrument server. Construct it with `New(Array<InstrumentPort>)` or deserialise from a `PORT_PAYLOAD` JSON string. It provides both element-wise access and bulk projections for names, connections, and knob/meter flags.

---

## API

```fal
// Constructor
routine New(Array<instrumentPort::InstrumentPort> ports) -> (Ports ports)

// Bulk accessors
routine Ports           -> (Array<instrumentPort::InstrumentPort> ports)
routine GetDefaultNames -> (Array<string> names)
routine GetPsuedoNames  -> (Array<connection::Connection> connections)
routine IsKnobs         -> (Array<bool> is_knobs)
routine IsMeters        -> (Array<bool> is_meters)

// Equality
routine Equal   (Ports other) -> (bool equal)
routine NotEqual(Ports other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (Ports ports)
```

---

## Example

```fal
import "libs/falconCore/instrumentInterfaces/names/ports/ports.fal";
import "libs/falconCore/instrumentInterfaces/names/instrumentPort/instrumentPort.fal";

InstrumentPort knob  = InstrumentPort.NewKnob("P1", p1, dc, volt, "plunger");
InstrumentPort meter = InstrumentPort.NewMeter("I1", i1, amm, amp, "ohmic");

Ports p = Ports.New([knob, meter]);

Array<string> names = p.GetDefaultNames();  // ["P1", "I1"]
Array<bool>   knobs = p.IsKnobs();          // [true, false]
Array<bool>   meters = p.IsMeters();        // [false, true]
```
