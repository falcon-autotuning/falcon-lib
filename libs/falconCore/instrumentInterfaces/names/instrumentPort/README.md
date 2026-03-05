# falcon/falconCore/instrumentInterfaces/names/instrumentPort

Falcon binding for `falcon_core::instrument_interfaces::names::InstrumentPort` — a single named port on a physical instrument, carrying type, unit, description, and device-facing connection metadata.

---

## Installation

```fal
import "libs/falconCore/instrumentInterfaces/names/instrumentPort/instrumentPort.fal";
```

---

## Dependencies

| Module | Path |
|--------|------|
| `connection` | `libs/falconCore/physics/deviceStructures/connection/connection.fal` |
| `symbolUnit` | `libs/falconCore/physics/units/symbolUnit/symbolUnit.fal` |
| `instrument` | `libs/falconCore/instrumentInterfaces/names/instrument/instrument.fal` |

---

## Overview

An `InstrumentPort` represents one controllable or measurable channel on the instrument server. Use `NewKnob` for a settable voltage/current output, `NewMeter` for a readable measurement input, or `NewTimer` / `NewExecutionClock` for timing ports.

---

## API

```fal
// Constructors
routine NewKnob (string default_name, connection::Connection conn,
                 string type, symbolUnit::SymbolUnit units, string description)
    -> (InstrumentPort port)

routine NewMeter (string default_name, connection::Connection conn,
                  string type, symbolUnit::SymbolUnit units, string description)
    -> (InstrumentPort port)

routine NewTimer         -> (InstrumentPort port)
routine NewExecutionClock -> (InstrumentPort port)

// Accessors
routine DefaultName          -> (string default_name)
routine PsuedoName           -> (connection::Connection conn)
routine InstrumentType       -> (string type)
routine Units                -> (symbolUnit::SymbolUnit units)
routine Description          -> (string description)
routine InstrumentFacingName -> (string facing_name)
routine IsKnob               -> (bool is_knob)
routine IsMeter              -> (bool is_meter)

// Equality
routine Equal   (InstrumentPort other) -> (bool equal)
routine NotEqual(InstrumentPort other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (InstrumentPort port)
```

---

## Example

```fal
import "libs/falconCore/instrumentInterfaces/names/instrumentPort/instrumentPort.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";
import "libs/falconCore/instrumentInterfaces/names/instrument/instrument.fal";

Connection  p1   = Connection.NewPlungerGate("P1");
SymbolUnit  volt = SymbolUnit.Volt();
string      dc   = Instrument.DCVoltageSource();

InstrumentPort knob  = InstrumentPort.NewKnob("P1", p1, dc, volt, "plunger gate");
bool is_knob         = knob.IsKnob();    // true
string name          = knob.DefaultName(); // "P1"

InstrumentPort timer = InstrumentPort.NewTimer();
```
