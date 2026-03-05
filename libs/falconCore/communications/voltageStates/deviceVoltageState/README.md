# falcon/falconCore/communications/voltageStates/deviceVoltageState

Falcon binding for `falcon_core::communications::voltage_states::DeviceVoltageState` — a voltage measurement on a named device connection, with units, full arithmetic, and serialisation.

---

## Installation

```fal
import "libs/falconCore/communications/voltageStates/deviceVoltageState/deviceVoltageState.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";
```

---

## Dependencies

| Module | Path |
|--------|------|
| `connection` | `libs/falconCore/physics/deviceStructures/connection/connection.fal` |
| `symbolUnit` | `libs/falconCore/physics/units/symbolUnit/symbolUnit.fal` |

---

## Overview

A `DeviceVoltageState` binds a physical voltage value (as a `float`) and its physical unit to a named `Connection` (gate or ohmic). It supports:

- **Construction** via `New(conn, voltage, unit)`
- **Accessors** for the stored connection, value, and unit
- **Unit conversion** (returns a new state scaled to the target unit)
- **Full arithmetic**: scale by `float`/`int`/`Quantity`, divide, raise to a power, add, subtract, negate, absolute value
- **Equality** comparison
- **JSON** serialisation / deserialisation

---

## API

```fal
// Constructor
routine New(connection::Connection conn, float voltage, symbolUnit::SymbolUnit unit)
    -> (DeviceVoltageState state)

// Accessors
routine Connection -> (connection::Connection conn)
routine Voltage    -> (float voltage)
routine Unit       -> (symbolUnit::SymbolUnit unit)

// Unit conversion
routine ConvertToUnit(symbolUnit::SymbolUnit new_unit) -> (DeviceVoltageState converted_state)

// Arithmetic — scale
routine Times  (float factor)         -> (DeviceVoltageState scaled_state)
routine Times  (int   factor)         -> (DeviceVoltageState scaled_state)
routine Times  (Quantity factor)      -> (DeviceVoltageState scaled_state)
routine Divides(float divisor)        -> (DeviceVoltageState scaled_state)
routine Divides(int   divisor)        -> (DeviceVoltageState scaled_state)
routine Divides(Quantity divisor)     -> (DeviceVoltageState scaled_state)
routine Power  (int exponent)         -> (DeviceVoltageState powered_state)

// Arithmetic — add / subtract
routine Add     (DeviceVoltageState other) -> (DeviceVoltageState sum_state)
routine Add     (int   other)              -> (DeviceVoltageState sum_state)
routine Add     (float other)              -> (DeviceVoltageState sum_state)
routine Subtract(DeviceVoltageState other) -> (DeviceVoltageState difference_state)
routine Subtract(int   other)              -> (DeviceVoltageState difference_state)
routine Subtract(float other)              -> (DeviceVoltageState difference_state)

// Unary
routine Negate -> (DeviceVoltageState negated_state)
routine Abs    -> (DeviceVoltageState absolute_state)

// Equality
routine Equal   (DeviceVoltageState other) -> (bool equal)
routine NotEqual(DeviceVoltageState other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (DeviceVoltageState state)
```

---

## Example

```fal
Connection  p1   = Connection.NewPlungerGate("P1");
SymbolUnit  volt = SymbolUnit.Volt();
DeviceVoltageState dvs = DeviceVoltageState.New(p1, 0.5, volt);

float v               = dvs.Voltage();           // 0.5
DeviceVoltageState s2 = dvs.Times(2.0);          // 1.0 V
DeviceVoltageState s3 = dvs.Negate();            // -0.5 V
DeviceVoltageState s4 = dvs.Add(0.1);            // 0.6 V

SymbolUnit mv         = SymbolUnit.Miilivolt();
DeviceVoltageState mv_state = dvs.ConvertToUnit(mv); // 500.0 mV

string json           = dvs.ToJSON();
DeviceVoltageState rt = DeviceVoltageState.FromJSON(json);
```
