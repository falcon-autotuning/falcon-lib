# falcon/falconCore/instrumentInterfaces/names/instrument

Falcon binding for `falcon_core::instrument_interfaces::names::Instrument` — a set of canonical instrument-type name constants used to identify hardware attached to the instrument server.

---

## Installation

```fal
import "libs/falconCore/instrumentInterfaces/names/instrument/instrument.fal";
```

---

## Overview

`Instrument` provides static string constants that identify the class of a physical instrument (e.g. DC voltage source, ammeter). These strings are used as keys when looking up or categorising `InstrumentPort` entries.

---

## API

All routines are static (no `this` argument) and return a `string`.

```fal
routine DCVoltageSource -> (string type)
routine Amnmeter        -> (string type)
routine Magnet          -> (string type)
routine VoltageSource   -> (string type)
routine Clock           -> (string type)
```

---

## Example

```fal
import "libs/falconCore/instrumentInterfaces/names/instrument/instrument.fal";

string dc  = Instrument.DCVoltageSource();
string amm = Instrument.Amnmeter();
string mag = Instrument.Magnet();
string vs  = Instrument.VoltageSource();
string clk = Instrument.Clock();
```
