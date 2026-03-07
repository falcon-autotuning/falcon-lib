# voltageStatesResponse

FAL binding for `falcon_core::communications::messages::VoltageStatesResponse`.

A `VoltageStatesResponse` is a communication message that carries a
`DeviceVoltageStates` collection alongside a human-readable status string,
used to report device voltage state results back to a caller.

## Usage

```fal
import "libs/falconCore/communications/messages/voltageStatesResponse/voltageStatesResponse.fal";
import "libs/falconCore/communications/voltageStates/deviceVoltageStates/deviceVoltageStates.fal";
import "libs/falconCore/communications/voltageStates/deviceVoltageState/deviceVoltageState.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";

Connection         c    = Connection.NewPlungerGate("P1");
SymbolUnit         volt = SymbolUnit.Volt();
DeviceVoltageState dvs  = DeviceVoltageState.New(c, 1.0, volt);
Array<DeviceVoltageState> arr = [dvs];
DeviceVoltageStates states    = DeviceVoltageStates.New(arr);

VoltageStatesResponse resp = VoltageStatesResponse.New("ok", states);

string             msg  = resp.Message();
DeviceVoltageStates got = resp.States();

bool eq   = resp.Equal(resp);
string json = resp.ToJSON();
VoltageStatesResponse resp2 = VoltageStatesResponse.FromJSON(json);
```

## Routines

| Routine | Description |
|---------|-------------|
| `New(message, states)` | Construct from a status string and `DeviceVoltageStates` |
| `States()` | Return the `DeviceVoltageStates` payload |
| `Message()` | Return the status message string |
| `Equal(other)` | Equality comparison |
| `NotEqual(other)` | Inequality comparison |
| `ToJSON()` | Serialize to JSON string |
| `FromJSON(json)` | Deserialize from JSON string |
