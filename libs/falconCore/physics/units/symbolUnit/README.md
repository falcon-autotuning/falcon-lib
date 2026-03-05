# falcon/falconCore/physics/units/symbolUnit

Falcon binding for `falconCore::physics::units::SymbolUnit` — a named, dimension-aware SI unit with symbol resolution.

---

## Installation

```fal
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";
```

---

## Overview

`SymbolUnit` wraps the underlying C++ `SymbolUnit` class which associates a physical `Unit` (dimension vector + scale) with a human-readable symbol string (e.g. `"V"`, `"mA"`, `"kHz"`).  Every named unit has a unique static factory and the instance exposes its symbol, name, equality, and JSON serialisation.

---

## API

### Factory constructors (no arguments)

| Routine | Description |
|---------|-------------|
| `SymbolUnit.Meter` | Metre (m) |
| `SymbolUnit.Kilogram` | Kilogram (kg) |
| `SymbolUnit.Second` | Second (s) |
| `SymbolUnit.Ampere` | Ampere (A) |
| `SymbolUnit.Kelvin` | Kelvin (K) |
| `SymbolUnit.Mole` | Mole (mol) |
| `SymbolUnit.Candela` | Candela (cd) |
| `SymbolUnit.Hertz` | Hertz (Hz) |
| `SymbolUnit.Newton` | Newton (N) |
| `SymbolUnit.Pascal` | Pascal (Pa) |
| `SymbolUnit.Joule` | Joule (J) |
| `SymbolUnit.Watt` | Watt (W) |
| `SymbolUnit.Coulomb` | Coulomb (C) |
| `SymbolUnit.Volt` | Volt (V) |
| `SymbolUnit.Farad` | Farad (F) |
| `SymbolUnit.Ohm` | Ohm (Ω) |
| `SymbolUnit.Siemens` | Siemens (S) |
| `SymbolUnit.Weber` | Weber (Wb) |
| `SymbolUnit.Tesla` | Tesla (T) |
| `SymbolUnit.Henry` | Henry (H) |
| `SymbolUnit.Minute` | Minute (min) |
| `SymbolUnit.Hour` | Hour (h) |
| `SymbolUnit.ElectronVolt` | Electron-volt (eV) |
| `SymbolUnit.Celsius` | Degree Celsius (°C) |
| `SymbolUnit.Fahrenheit` | Degree Fahrenheit (°F) |
| `SymbolUnit.Dimensionless` | Dimensionless scalar |
| `SymbolUnit.Percent` | Percent (%) |
| `SymbolUnit.Radian` | Radian (rad) |
| `SymbolUnit.Kilometer` | Kilometre (km) |
| `SymbolUnit.Miillimeter` | Millimetre (mm) |
| `SymbolUnit.Miilivolt` | Millivolt (mV) |
| `SymbolUnit.Kilovolt` | Kilovolt (kV) |
| `SymbolUnit.Milliampere` | Milliampere (mA) |
| `SymbolUnit.Microampere` | Microampere (µA) |
| `SymbolUnit.Nanoampere` | Nanoampere (nA) |
| `SymbolUnit.Picoampere` | Picoampere (pA) |
| `SymbolUnit.Millisecond` | Millisecond (ms) |
| `SymbolUnit.Microsecond` | Microsecond (µs) |
| `SymbolUnit.Nanosecond` | Nanosecond (ns) |
| `SymbolUnit.Picosecond` | Picosecond (ps) |
| `SymbolUnit.Milliohm` | Milliohm (mΩ) |
| `SymbolUnit.Kiloohm` | Kilohm (kΩ) |
| `SymbolUnit.Megaohm` | Megaohm (MΩ) |
| `SymbolUnit.Millihertz` | Millihertz (mHz) |
| `SymbolUnit.Kilohertz` | Kilohertz (kHz) |
| `SymbolUnit.Megahertz` | Megahertz (MHz) |
| `SymbolUnit.Gigahertz` | Gigahertz (GHz) |
| `SymbolUnit.Meterspersecond` | Metres per second (m/s) |
| `SymbolUnit.MeterspersecondSquared` | Metres per second² (m/s²) |
| `SymbolUnit.Newtonmeter` | Newton-metre (N·m) |
| `SymbolUnit.Newtonpermeter` | Newton per metre (N/m) |
| `SymbolUnit.Voltspermeter` | Volts per metre (V/m) |
| `SymbolUnit.Voltspersecond` | Volts per second (V/s) |
| `SymbolUnit.Amperespermeter` | Amperes per metre (A/m) |
| `SymbolUnit.Voltsperampere` | Volts per ampere (V/A = Ω) |
| `SymbolUnit.Wattspermeterkelvin` | Watts per metre·kelvin (W/(m·K)) |

### Instance methods

```fal
routine Name                                     -> (string name)
routine Symbol                                   -> (string symbol)
routine IsEqual     (SymbolUnit other)            -> (bool equal)
routine IsNotEqual  (SymbolUnit other)            -> (bool notequal)
routine ToJSON      ()                            -> (string json)
routine FromJSON    (string json)                 -> (SymbolUnit unit)
```

---

## Example

```fal
import "libs/falconCore/physics/units/symbolUnit/symbolUnit.fal";

SymbolUnit v  = SymbolUnit.Volt();
SymbolUnit mv = SymbolUnit.Miilivolt();

string sym  = v.Symbol();   // "V"
string name = v.Name();     // "volt"
bool   eq   = v.IsEqual(mv);  // false
```
