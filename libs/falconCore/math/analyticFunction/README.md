# falcon/falconCore/math/analyticFunction

Falcon binding for `falcon_core::math::AnalyticFunction` — a named, serialisable mathematical expression parameterised by a labelled vector of variables.

---

## Installation

```fal
import "libs/falconCore/math/analyticFunction/analyticFunction.fal";
```

---

## Dependencies

| Module | Path |
|--------|------|
| `array` | `libs/falconCore/collections/array/array.fal` |

---

## Overview

`AnalyticFunction` represents a symbolic mathematical expression (e.g. `"x[0]*t^2 + x[1]"`) together with an ordered list of human-readable labels for its vector arguments.  Two convenience factories — `Identity` and `Constant` — cover common single-variable cases.

---

## API

```fal
// Constructors
routine New     (Array<string> labels, string expression) -> (AnalyticFunction out)
routine Identity()                                        -> (AnalyticFunction out)
routine Constant(float value)                             -> (AnalyticFunction out)

// Accessor
routine Labels  ()                       -> (Array<string> variables)

// Equality
routine Equal   (AnalyticFunction other) -> (bool equal)
routine NotEqual(AnalyticFunction other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (AnalyticFunction dstate)
```

---

## Example

```fal
import "libs/falconCore/math/analyticFunction/analyticFunction.fal";

AnalyticFunction f = AnalyticFunction.New(["amplitude", "offset"], "x[0]*t + x[1]");
Array<string> lbls = f.Labels();    // ["amplitude", "offset"]

AnalyticFunction id = AnalyticFunction.Identity();
AnalyticFunction c  = AnalyticFunction.Constant(3.14);

string json = f.ToJSON();
AnalyticFunction f2 = AnalyticFunction.FromJSON(json);
bool same = f.Equal(f2);            // true
```
