# acquisitionContext

FAL binding for `falcon_core::autotuner_interfaces::contexts::AcquisitionContext`.

An `AcquisitionContext` extends a `MeasurementContext` with a physical unit,
fully describing a single measurement acquisition: which connection, which
instrument, and in what unit the result is expressed.

## Usage

```fal
import "libs/falconCore/autotunerInterfaces/contexts/acquisitionContext/acquisitionContext.fal";
import "libs/falconCore/autotunerInterfaces/contexts/measurementContext/measurementContext.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
import "libs/falconCore/instrumentInterfaces/names/instrument/instrument.fal";
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";

Connection         conn = Connection.NewPlungerGate("P1");
string             inst = Instrument.DCVoltageSource();
SymbolUnit         volt = SymbolUnit.Volt();
AcquisitionContext ac   = AcquisitionContext.New(conn, inst, volt);

// Construct from an existing MeasurementContext
MeasurementContext mc   = MeasurementContext.New(conn, inst);
AcquisitionContext ac2  = AcquisitionContext.NewFromContext(mc, volt);

Connection got_conn     = ac.Connection();
string     got_inst     = ac.Instrument();
SymbolUnit got_unit     = ac.Units();

bool eq = ac.Equal(ac2);
string json = ac.ToJSON();
AcquisitionContext ac3 = AcquisitionContext.FromJSON(json);
```

## Routines

| Routine | Description |
|---------|-------------|
| `New(conn, inst, unit)` | Construct from connection, instrument type, and unit |
| `NewFromContext(ctx, unit)` | Construct from a `MeasurementContext` and unit |
| `Connection()` | Return the device connection |
| `Instrument()` | Return the instrument type string |
| `Units()` | Return the measurement unit |
| `Division(other: AcquisitionContext)` | Divide units; keep top context details |
| `Division(other: SymbolUnit)` | Divide by a unit |
| `MatchConnection(conn)` | Check if connection matches |
| `MatchInstrumentType(inst)` | Check if instrument type matches |
| `Equal(other)` | Equality comparison |
| `NotEqual(other)` | Inequality comparison |
| `ToJSON()` | Serialize to JSON string |
| `FromJSON(json)` | Deserialize from JSON string |
