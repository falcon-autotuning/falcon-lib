# falcon/falconCore/communications/messages/standardResponse

Falcon binding for `falcon_core::communications::messages::StandardResponse` — a simple string-carrying response message.

---

## Installation

```fal
import "libs/falconCore/communications/messages/standardResponse/standardResponse.fal";
```

---

## Overview

A `StandardResponse` wraps a single string payload used as a communication response.  It supports equality comparison and JSON serialisation.

---

## API

```fal
// Constructor
routine New(string message) -> (StandardResponse response)

// Accessor
routine Message -> (string message)

// Equality
routine Equal   (StandardResponse other) -> (bool equal)
routine NotEqual(StandardResponse other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (StandardResponse response)
```

---

## Example

```fal
import "libs/falconCore/communications/messages/standardResponse/standardResponse.fal";

StandardResponse resp = StandardResponse.New("ok");
string           msg  = resp.Message();          // "ok"
bool             same = resp.Equal(resp);         // true
string           json = resp.ToJSON();
StandardResponse resp2 = StandardResponse.FromJSON(json);
bool             rt   = resp.Equal(resp2);        // true
```
