# falcon/falconCore/instrumentInterfaces/portTransforms/portTransforms

Falcon binding for `falcon_core::instrument_interfaces::port_transforms::PortTransforms` — a collection of `PortTransform` objects.

## Import

```fal
import "libs/falconCore/instrumentInterfaces/portTransforms/portTransforms/portTransforms.fal";
```

## API

| Routine | Description |
|---------|-------------|
| `New(Array<PortTransform>)` | Construct from array of transforms |
| `Size()` | Number of transforms |
| `IsEmpty()` | True if no transforms |
| `GetIndex(int)` | Get transform at index |
| `SetIndex(int, PortTransform)` | Set transform at index |
| `PushBack(PortTransform)` | Append a transform |
| `PopBack()` | Remove and return last transform |
| `Insert(int, PortTransform)` | Insert at index |
| `Erase(int)` | Remove at index |
| `Clear()` | Remove all transforms |
| `Contains(PortTransform)` | Check if transform exists |
| `IndexOf(PortTransform)` | Find index of transform |
| `Equal(PortTransforms)` | Equality comparison |
| `NotEqual(PortTransforms)` | Inequality comparison |
| `ToJSON()` | Serialize to JSON string |
| `FromJSON(string)` | Deserialize from JSON string |
