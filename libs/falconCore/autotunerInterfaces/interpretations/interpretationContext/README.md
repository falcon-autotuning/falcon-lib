# interpretationContext

FAL binding for `falcon_core::autotuner_interfaces::interpretations::InterpretationContext`.

An `InterpretationContext` fully describes a multi-dimensional measurement
sweep: a set of independent variables (the sweep axes), a set of dependent
variables (what is measured), and the physical unit for interpreting the
result values.

## Usage

```fal
import "libs/falconCore/autotunerInterfaces/interpretations/interpretationContext/interpretationContext.fal";
import "libs/falconCore/autotunerInterfaces/contexts/measurementContext/measurementContext.fal";
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";
import "libs/falconCore/physics/deviceStructures/connection/connection.fal";
import "libs/falconCore/instrumentInterfaces/names/instrument/instrument.fal";
import "libs/falconCore/math/axes/axes.fal";
import "libs/collections/array/array.fal";

Connection conn_p1 = Connection.NewPlungerGate("P1");
string     dc      = Instrument.DCVoltageSource();
string     amm     = Instrument.Amnmeter();
SymbolUnit volt    = SymbolUnit.Volt();

MeasurementContext indep = MeasurementContext.New(conn_p1, dc);
MeasurementContext dep   = MeasurementContext.New(conn_p1, amm);

Axes<MeasurementContext>  indep_axes = Axes<MeasurementContext>.New();
indep_axes.PushBack(indep);
Array<MeasurementContext> dep_arr = [dep];

InterpretationContext ic = InterpretationContext.New(indep_axes, dep_arr, volt);

int dim       = ic.Dimension();
SymbolUnit u  = ic.Unit();
MeasurementContext got = ic.GetIndependentVariable(0);

ic.AddDependentVariable(dep);
ic.ReplaceDependantVariable(0, dep);

InterpretationContext ic2 = ic.WithUnit(SymbolUnit.Ampere());

bool eq   = ic.Equal(ic2);
string json = ic.ToJSON();
InterpretationContext ic3 = InterpretationContext.FromJSON(json);
```

## Routines

| Routine | Description |
|---------|-------------|
| `New(independantVariables, dependantVariables, unit)` | Construct with axes, dependent vars, and unit |
| `IndependentVariables()` | Return independent variable axes as an array |
| `DependentVariables()` | Return dependent variable list as an array |
| `Unit()` | Return the measurement unit |
| `Dimension()` | Return the number of independent variables |
| `AddDependentVariable(context)` | Append a dependent variable |
| `ReplaceDependantVariable(index, context)` | Replace a dependent variable by index |
| `GetIndependentVariable(index)` | Get an independent variable by index |
| `WithUnit(unit)` | Return a new context with a different unit |
| `Equal(other)` | Equality comparison |
| `NotEqual(other)` | Inequality comparison |
| `ToJSON()` | Serialize to JSON string |
| `FromJSON(json)` | Deserialize from JSON string |
