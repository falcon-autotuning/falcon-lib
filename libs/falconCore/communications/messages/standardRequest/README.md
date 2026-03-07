# falcon/falconCore/communications/messages/standardRequest

Falcon binding for `falcon_core::communications::messages::StandardRequest` — a simple string-carrying request message.

---

## Installation

```fal
import "libs/falconCore/communications/messages/standardRequest/standardRequest.fal";
```

---

## Overview

A `StandardRequest` wraps a single string payload used as a communication request.  It supports equality comparison and JSON serialisation.

---

## API

```fal
// Constructor
routine New(string message) -> (StandardRequest request)

// Accessor
routine Message -> (string message)

// Equality
routine Equal   (StandardRequest other) -> (bool equal)
routine NotEqual(StandardRequest other) -> (bool notequal)

// Serialisation
routine ToJSON  ()            -> (string json)
routine FromJSON(string json) -> (StandardRequest request)
```

---

## Example

```fal
import "libs/falconCore/communications/messages/standardRequest/standardRequest.fal";

StandardRequest req  = StandardRequest.New("hello");
string          msg  = req.Message();          // "hello"
bool            same = req.Equal(req);         // true
string          json = req.ToJSON();
StandardRequest req2 = StandardRequest.FromJSON(json);
bool            rt   = req.Equal(req2);        // true
```
