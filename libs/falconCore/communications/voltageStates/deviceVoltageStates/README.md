# falcon/falconCore/communications/voltageStates/deviceVoltageStates

Falcon binding for `falcon_core::communications::voltage_states::DeviceVoltageStates` — an ordered, mutable collection of `DeviceVoltageState` entries, keyed by `Connection`.

---

## Installation

```fal
import "libs/falconCore/communications/voltageStates/deviceVoltageStates/deviceVoltageStates.fal";
import "libs/falconCore/communications/voltageStates/deviceVoltageState/deviceVoltageState.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";
```

---

## Dependencies

| Module | Path |
|--------|------|
| `deviceVoltageState` | `libs/falconCore/communications/voltageStates/deviceVoltageState/deviceVoltageState.fal` |
| `connection` | `libs/falconCore/physics/deviceStructures/connection/connection.fal` |
| `symbolUnit` | `libs/falconCore/physics/units/symbolUnit/symbolUnit.fal` |

---

## Overview

`DeviceVoltageStates` holds the complete voltage configuration for a sample — one `DeviceVoltageState` per gate/ohmic connection. It supports:

- **Construction** from an `Array<DeviceVoltageState>` via `New`
- **Mutation** via `AddState`, `Insert`, `Clear`
- **Query** via `FindState`, `Contains`, `Size`, `Empty`
- **Projection** to a `Point` (for optimisers) via `ToPoint`
- **Equality** and **JSON** serialisation

---

## API

```fal
// Constructor
routine New(Array<DeviceVoltageState> states) -> (DeviceVoltageStates states)

// Mutators
routine AddState(DeviceVoltageState state)              -> ()
routine Insert  (connection::Connection conn, int loc)  -> ()
routine Clear   ()                                      -> ()

// Accessors
routine FindState(connection::Connection conn) -> (DeviceVoltageState state)
routine Contains (connection::Connection conn) -> (bool contains)
routine Size     ()                            -> (int size)
routine Empty    ()                            -> (bool isEmpty)
routine ToPoint  ()                            -> (point::Point point)

// Equality
routine Equal   (DeviceVoltageStates other) -> (bool equal)
routine NotEqual(DeviceVoltageStates other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (DeviceVoltageStates states)
```

---

## Example

```fal
Connection p1  = Connection.NewPlungerGate("P1");
Connection b1  = Connection.NewBarrierGate("B1");
SymbolUnit v   = SymbolUnit.Volt();

DeviceVoltageState s1 = DeviceVoltageState.New(p1, 0.5, v);
DeviceVoltageState s2 = DeviceVoltageState.New(b1, 0.2, v);

DeviceVoltageStates states = DeviceVoltageStates.New([s1, s2]);

bool has_p1 = states.Contains(p1);                  // true
int  sz     = states.Size();                         // 2
DeviceVoltageState found = states.FindState(p1);     // 0.5 V on P1

states.Clear();
bool empty = states.Empty();                         // true
```
