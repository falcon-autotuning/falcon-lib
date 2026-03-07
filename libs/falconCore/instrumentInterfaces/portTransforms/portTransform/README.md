# falcon/falconCore/instrumentInterfaces/portTransforms/portTransform

Falcon binding for `falcon_core::instrument_interfaces::port_transforms::PortTransform` — a transform that maps an `InstrumentPort` to a time-domain analytic function.

---

## Installation

```fal
import "libs/falconCore/instrumentInterfaces/portTransforms/portTransform/portTransform.fal";
```

---

## Dependencies

| Module | Path |
|--------|------|
| `instrumentPort` | `libs/falconCore/instrumentInterfaces/names/instrumentPort/instrumentPort.fal` |
| `analyticFunction` | `libs/falconCore/math/analyticFunction/analyticFunction.fal` |

---

## Overview

`PortTransform` associates an `InstrumentPort` with an `AnalyticFunction` describing how the port's value is transformed over time. Factory helpers `ConstantTransform` and `IdentityTranform` cover the two most common cases without needing to supply a full analytic function.

---

## API

```fal
// Constructors
routine New(InstrumentPort port, AnalyticFunction func) -> (PortTransform transform)
routine ConstantTransform(InstrumentPort port, float value)   -> (PortTransform transform)
routine IdentityTranform(InstrumentPort port)                  -> (PortTransform transform)

// Accessors
routine Port()   -> (InstrumentPort port)
routine Labels() -> (Array<string> variables)

// Equality
routine Equal(PortTransform other)    -> (bool equal)
routine NotEqual(PortTransform other) -> (bool notequal)

// Serialisation
routine ToJSON                  -> (string json)
routine FromJSON(string json)   -> (PortTransform transform)
```

---

## Example

```fal
import "libs/falconCore/instrumentInterfaces/names/instrumentPort/instrumentPort.fal";
import "libs/falconCore/instrumentInterfaces/portTransforms/portTransform/portTransform.fal";

// Build a constant transform that always outputs 1.5
InstrumentPort port = InstrumentPort.NewKnob("VG", conn, "DC", unit, "gate voltage");
PortTransform  pt   = PortTransform.ConstantTransform(port, 1.5);

// Round-trip via JSON
string        json  = pt.ToJSON();
PortTransform pt2   = PortTransform.FromJSON(json);
bool          same  = pt.Equal(pt2);   // true
```
