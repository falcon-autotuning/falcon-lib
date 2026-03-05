# falcon/falconCore/communications/voltageStates/deviceVoltageState

Falcon binding for `falconCore::communications::voltage_states::DeviceVoltageState` — a voltage quantity attached to a named device `Connection` with full dimensional-unit support.

---

## Installation

```fal
import "libs/falconCore/communications/voltageStates/deviceVoltageState/deviceVoltageState.fal";
```

---

## Dependencies

| Module | Path |
|--------|------|
| `connection` | `libs/falconCore/physics/deviceStructures/connection/connection.fal` |
| `symbolUnit` | `libs/falconCore/physics/units/symbolUnit/symbolUnit.fal` |

---

## Overview

A `DeviceVoltageState` bundles:

- a **`Connection`** (which terminal this voltage applies to),
- a **voltage** (floating-point value),
- a **`SymbolUnit`** (the physical unit of the voltage value, e.g. `Volt`, `MilliVolt`).

It supports full arithmetic (scale, power, add, subtract, negate, abs) and in-place unit conversion.

---

## API

```fal
// Constructor
routine New          (Connection conn, float voltage, SymbolUnit unit)
                     -> (DeviceVoltageState state)

// Accessors
routine Connection   -> (Connection conn)
routine Voltage      -> (float voltage)
routine Unit         -> (SymbolUnit unit)

// Unit conversion (returns a new state in the target unit)
routine ConvertToUnit(SymbolUnit new_unit) -> (DeviceVoltageState converted_state)

// Arithmetic — scale
routine Times  (float factor)            -> (DeviceVoltageState scaled_state)
routine Times  (int   factor)            -> (DeviceVoltageState scaled_state)
routine Times  (Quantity factor)         -> (DeviceVoltageState scaled_state)
routine Divides(float divisor)           -> (DeviceVoltageState scaled_state)
routine Divides(int   divisor)           -> (DeviceVoltageState scaled_state)
routine Divides(Quantity divisor)        -> (DeviceVoltageState scaled_state)
routine Power  (float exponent)          -> (DeviceVoltageState powered_state)

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
routine ToJSON  ()           -> (string json)
routine FromJSON(string json) -> (DeviceVoltageState state)
```

---

## Example

```fal
import "libs/falconCore/communications/voltageStates/deviceVoltageState/deviceVoltageState.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";

Connection  p1   = Connection.NewPlungerGate("P1");
SymbolUnit  volt = SymbolUnit.Volt();
DeviceVoltageState dvs = DeviceVoltageState.New(p1, 0.5, volt);

float v = dvs.Voltage();                        // 0.5
DeviceVoltageState scaled  = dvs.Times(2.0);    // 1.0 V on P1
DeviceVoltageState negated = dvs.Negate();      // -0.5 V on P1
DeviceVoltageState added   = dvs.Add(0.1);      // 0.6 V on P1

SymbolUnit mv = SymbolUnit.Miilivolt();
DeviceVoltageState in_mv = dvs.ConvertToUnit(mv); // 500.0 mV on P1
```
