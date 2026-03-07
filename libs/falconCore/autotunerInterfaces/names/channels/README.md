# falcon/falconCore/autotunerInterfaces/names/channels

Falcon binding for `falcon_core::autotuner_interfaces::names::Channels` — an ordered, serialisable collection of `Channel` objects.

---

## Installation

```fal
import "libs/falconCore/autotunerInterfaces/names/channels/channels.fal";
```

---

## Dependencies

| Module | Path |
|--------|------|
| `channel` | `libs/falconCore/autotunerInterfaces/names/channel/channel.fal` |
| `array` | `libs/falconCore/collections/array/array.fal` |

---

## Overview

`Channels` is a `List<Channel>` that provides construction from an array of `Channel` objects together with full list-mutation and query operations.

---

## API

```fal
// Constructor
routine New(Array<Channel> array) -> (Channels channels)

// Equality
routine Equal   (Channels other) -> (bool equal)
routine NotEqual(Channels other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (Channels channel)

// List operations
routine Size     ()                        -> (int size)
routine IsEmpty  ()                        -> (bool empty)
routine GetIndex (int index)               -> (Channel value)
routine SetIndex (int index, Channel value) -> ()
routine PushBack (Channel value)           -> ()
routine PopBack  ()                        -> (Channel value)
routine Insert   (int index, Channel value) -> ()
routine Erase    (int index)               -> ()
routine Clear    ()                        -> ()
routine Contains (Channel value)           -> (bool found)
routine IndexOf  (Channel value)           -> (int index)
```

---

## Example

```fal
import "libs/falconCore/autotunerInterfaces/names/channels/channels.fal";
import "libs/falconCore/autotunerInterfaces/names/channel/channel.fal";

Channel c1 = Channel.New("ch1");
Channel c2 = Channel.New("ch2");

Channels chs = Channels.New([c1, c2]);
int  sz      = chs.Size();          // 2
bool has_c1  = chs.Contains(c1);    // true
int  idx     = chs.IndexOf(c2);     // 1

Channel popped = chs.PopBack();     // c2, size becomes 1

string json    = chs.ToJSON();
Channels chs2  = Channels.FromJSON(json);
bool same      = chs.Equal(chs2);   // true
```
