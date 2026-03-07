# falcon/falconCore/autotunerInterfaces/names/channel

Falcon binding for `falcon_core::autotuner_interfaces::names::Channel` — a named channel identifier used in autotuner interfaces.

---

## Installation

```fal
import "libs/falconCore/autotunerInterfaces/names/channel/channel.fal";
```

---

## Overview

A `Channel` wraps a single string name used to identify an autotuner channel.  It supports equality comparison and JSON serialisation.

---

## API

```fal
// Constructor
routine New(string name) -> (Channel channel)

// Accessor
routine Name -> (string name)

// Equality
routine Equal   (Channel other) -> (bool equal)
routine NotEqual(Channel other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (Channel channel)
```

---

## Example

```fal
import "libs/falconCore/autotunerInterfaces/names/channel/channel.fal";

Channel c = Channel.New("ch1");
string name = c.Name();           // "ch1"
bool same   = c.Equal(c);         // true
string json = c.ToJSON();
Channel c2  = Channel.FromJSON(json);
bool rt     = c.Equal(c2);        // true
```
