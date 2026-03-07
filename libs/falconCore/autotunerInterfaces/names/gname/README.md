# falcon/falconCore/autotunerInterfaces/names/gname

Falcon binding for `falcon_core::autotuner_interfaces::names::Gname` — a group-name identifier that can be constructed from either a string or an integer.

---

## Installation

```fal
import "libs/falconCore/autotunerInterfaces/names/gname/gname.fal";
```

---

## Overview

A `Gname` represents a named group used in autotuner interfaces.  It can be initialised from a string label or an integer index and exposes equality comparison and JSON serialisation.

---

## API

```fal
// Constructors (both overloads share the same routine name)
routine New(string name) -> (Gname gname)
routine New(int    name) -> (Gname gname)

// Accessor
routine gname -> (string out)

// Equality
routine Equal   (Gname other) -> (bool equal)
routine NotEqual(Gname other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (Gname gname)
```

---

## Example

```fal
import "libs/falconCore/autotunerInterfaces/names/gname/gname.fal";

Gname g1 = Gname.New("group1");
Gname g2 = Gname.New(42);
string s1   = g1.gname();          // "group1"
string s2   = g2.gname();          // integer-based name string
bool   same = g1.Equal(g1);        // true
bool   diff = g1.NotEqual(g2);     // true
string json = g1.ToJSON();
Gname  g3   = Gname.FromJSON(json);
bool   rt   = g1.Equal(g3);        // true
```
