# falcon/falconCore/instrumentInterfaces/names/ports

Falcon binding for `falcon_core::instrument_interfaces::names::Ports` — a collection of all `InstrumentPort` entries reported by the instrument server, with bulk accessor helpers.

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

`Ports` is the top-level collection object that the instrument server returns in a `PORT_PAYLOAD` message. It provides both element-wise access (as an `Array<InstrumentPort>`) and bulk projection helpers for names, connections, and knob/meter flags.

---

## API

```fal
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

Ports p = Ports.FromJSON(payload_json);

Array<string>  names  = p.GetDefaultNames();
Array<bool>    knobs  = p.IsKnobs();
Array<bool>    meters = p.IsMeters();
```
