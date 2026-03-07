# LabelledDomain

A `Domain` (numeric interval) associated with an `InstrumentPort`, binding
measurement bounds to a specific instrument channel.

## Construction

```falcon
import "libs/falconCore/math/domains/labelledDomain/labelledDomain.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";
import "libs/falconCore/instrumentInterfaces/names/instrument/instrument.fal";

Connection p1   = Connection.NewPlungerGate("P1");
SymbolUnit volt = SymbolUnit.Volt();
string dc       = Instrument.DCVoltageSource();

LabelledDomain ld = LabelledDomain.NewFromKnob(
    "P1", -0.5, 0.5, p1, dc, true, true, volt, "plunger gate");
```

## Constructors

| Routine | Description |
|---|---|
| `NewFromKnob(defaultName, min, max, psuedoName, instrumentType, lesserBoundContained, greaterBoundContained, units, description)` | Build from explicit bounds and connection metadata |
| `FromPort(max, min, port, lesserBoundContained, greaterBoundContained)` | Build from an `InstrumentPort` and bounds |
| `FromPortAndDomain(port, domain)` | Build from a port and an existing `Domain` |
| `FromDomain(domain, defaultName, psuedoName, instrumentType, units, description)` | Build from an existing `Domain` with labels |

## Accessors

| Routine | Returns |
|---|---|
| `Port()` | `InstrumentPort` — the associated port |
| `Domain()` | `Domain` — the underlying numeric interval |
| `MatchingPort(port)` | `bool` — true if the port matches |
| `LesserBound()` | `float` |
| `GreaterBound()` | `float` |
| `LesserBoundContained()` | `bool` |
| `GreaterBoundContained()` | `bool` |
| `In(value)` | `bool` — true if value is inside the domain |
| `Range()` | `float` — width of the domain |
| `GetCenter()` | `float` — midpoint |

## Operations

`Intersection`, `Union`, `ContainsDomain`, `Shift`, `Scaled`, `Transform`,
`Equal`, `NotEqual`, `ToJSON`, `FromJSON`.
