# falcon/falconCore/autotunerInterfaces/interpretations/interpretationContainer

Falcon binding for `falcon_core::autotuner_interfaces::interpretations::InterpretationContainer<T>` — a map from `InterpretationContext` keys to typed values `T`, where all contexts must share the same `SymbolUnit`.

---

## Installation

```fal
import "libs/falconCore/autotunerInterfaces/interpretations/interpretationContainer/interpretationContainer.fal";
```

---

## Overview

`InterpretationContainer<T>` extends `Map<InterpretationContext, T>`.  All contexts stored in the container must carry the same `SymbolUnit`; the unit is enforced at construction time.  The binding monomorphises `T = double` on the C++ side.

The container exposes selection helpers that filter contexts by which `Connection` appears in their independent or dependent variables, making it easy to slice a multi-dimensional interpretation result.

---

## API

```fal
// Constructor — build from a non-empty Map<InterpretationContext, T>
routine New(map: Map<InterpretationContext, T>) -> (InterpretationContainer container)

// Unit shared by all contexts
routine Unit() -> (SymbolUnit unit)

// Context selection
routine SelectByConnection            (conn: Connection) -> (Array<InterpretationContext> contexts)
routine SelectByConnections           (conn: Connection) -> (Array<InterpretationContext> contexts)
routine SelectByIndependentConnection (conn: Connection) -> (Array<InterpretationContext> contexts)
routine SelectByDependentConnection   (conn: Connection) -> (Array<InterpretationContext> contexts)
routine SelectContexts(independentConnections: Array<Connection>,
                       dependentConnections:   Array<Connection>) -> (Array<InterpretationContext> contexts)

// Equality
routine Equal   (other: InterpretationContainer) -> (bool equal)
routine NotEqual(other: InterpretationContainer) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (InterpretationContainer cont)
```

---

## Example

```fal
import "libs/falconCore/autotunerInterfaces/interpretations/interpretationContainer/interpretationContainer.fal";
import "libs/falconCore/autotunerInterfaces/interpretations/interpretationContext/interpretationContext.fal";
import "libs/falconCore/collections/map/map.fal";

// Deserialise from a previously serialised container
string json = "...";
interpretationContainer::InterpretationContainer ic = interpretationContainer::InterpretationContainer.FromJSON(json);

// Query unit
symbolUnit::SymbolUnit unit = ic.Unit();

// Select all contexts that touch a specific connection
connection::Connection gate = Connection.NewPlungerGate("P1");
Array<interpretationContext::InterpretationContext> ctxs = ic.SelectByConnection(gate);

// Round-trip check
string json2 = ic.ToJSON();
interpretationContainer::InterpretationContainer ic2 = interpretationContainer::InterpretationContainer.FromJSON(json2);
bool same = ic.Equal(ic2);
```

---

## Notes

- The `New` constructor requires a **non-empty** `Map<InterpretationContext, T>` — the first key's unit is used to initialise the container's shared unit.  Passing an empty map will throw at runtime.
- The binding monomorphises `T` to `double`; the generic `<T>` type parameter in the `.fal` declaration is resolved to `double` by the C++ wrapper.
