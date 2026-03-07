# measurementContext

FAL binding for `falcon_core::autotuner_interfaces::contexts::MeasurementContext`.

A `MeasurementContext` pairs a physical device connection with an instrument
type, identifying a single measurement point in an autotuner pipeline.

## Usage

```fal
import "libs/falconCore/autotunerInterfaces/contexts/measurementContext/measurementContext.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
import "libs/falconCore/instrumentInterfaces/names/instrument/instrument.fal";

Connection   conn = Connection.NewPlungerGate("P1");
string       inst = Instrument.DCVoltageSource();
MeasurementContext mc = MeasurementContext.New(conn, inst);

Connection c2    = mc.Connection();
string     itype = mc.Instrument();

bool eq = mc.Equal(mc);
string json = mc.ToJSON();
MeasurementContext mc2 = MeasurementContext.FromJSON(json);
```

## Routines

| Routine | Description |
|---------|-------------|
| `New(conn, inst)` | Construct from a `Connection` and instrument type string |
| `Connection()` | Return the device connection |
| `Instrument()` | Return the instrument type string |
| `Equal(other)` | Equality comparison |
| `NotEqual(other)` | Inequality comparison |
| `ToJSON()` | Serialize to JSON string |
| `FromJSON(json)` | Deserialize from JSON string |
