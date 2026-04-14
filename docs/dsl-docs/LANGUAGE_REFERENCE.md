# Falcon DSL Language Reference

## Table of Contents

1. [Overview](#overview)
2. [File Structure](#file-structure)
3. [Syntax Rules](#syntax-rules)
4. [Types](#types)
5. [Autotuner Declaration](#autotuner-declaration)
6. [Variable Declarations](#variable-declarations)
7. [States](#states)
8. [Statements](#statements)
9. [Transitions](#transitions)
10. [Control Flow](#control-flow)
11. [Expressions](#expressions)
12. [Routines](#routines)
13. [Structs](#structs)
14. [Generic Structs](#generic-structs)
15. [Imports and Modules](#imports-and-modules)
16. [FFI (Foreign Function Interface)](#ffi-foreign-function-interface)
17. [Complete Examples](#complete-examples)
18. [Common Mistakes](#common-mistakes)
19. [Grammar Summary](#grammar-summary)

---

## Overview

The Falcon DSL (`.fal`) is a domain-specific language for defining state machines used in quantum device autotuning. A `.fal` file is parsed by the Falcon compiler and executed by the `AutotunerEngine` runtime.

Key design principles:

- **Typed inputs and outputs** — autotuners and routines declare explicit typed signatures
- **State machine control flow** — execution is a sequence of named states; transitions are explicit
- **Composability** — routines and structs can be defined in separate files and imported
- **Generic structs** — structs can be parameterised by type, enabling reusable data structures
- **FFI** — C++ hardware routines can be bound and called directly from `.fal` code

---

## File Structure

A `.fal` file may contain, in order:

1. Zero or more `import` statements
2. Zero or more `ffimport` declarations
3. Any mix of `struct`, `routine`, and `autotuner` declarations

```fal
// 1. Imports (must come first)
import "shared/types.fal";

// 2. FFI bindings (optional)
ffimport "hardware.cpp" ("-I/opt/hw/include") ("-lhardware")

// 3. Struct definitions (may be generic)
struct Config {
    float threshold = 0.5;
}

struct Box <T> {
    T value;
    routine New (T v) -> (Box<T> b) { b.value = v; }
    routine Get  -> (T out)         { out = this.value; }
}

// 4. Routine definitions
routine clamp (float v, float lo, float hi) -> (float out) {
    if (v < lo) { out = lo; }
    elif (v > hi) { out = hi; }
    else { out = v; }
}

// 5. Autotuner definitions
autotuner MyAutotuner (float input) -> (float result) {
    result = 0.0;
    start -> run;

    state run {
        result = clamp(input, 0.0, 1.0);
        -> done;
    }

    state done { terminal; }
}
```

---

## Syntax Rules

### Case sensitivity

- Keywords are lowercase: `autotuner`, `routine`, `struct`, `state`, `start`, `terminal`, `if`, `elif`, `else`, `import`, `ffimport`, `this`, `nil`
- Identifiers are case-sensitive: `myState ≠ MyState`

> **Note:** `uses` is **no longer a keyword** and must not appear in `.fal` source. Cross-module routines are called directly with their qualified name — no declaration is needed.

### Naming conventions

| Thing | Convention | Examples |
|-------|-----------|---------|
| Autotuner names | PascalCase | `VoltageSweep`, `ChargeStability` |
| State names | snake_case | `initialize`, `sweep_loop` |
| Variable/param names | snake_case | `min_voltage`, `step_count` |
| Routine names | PascalCase or snake_case | `Clamp`, `area_square` |
| Struct names | PascalCase | `SweepConfig`, `DeviceState` |
| Generic type params | single uppercase letter or short PascalCase | `T`, `K`, `V`, `Elem` |

### Delimiters

- Statements end with `;`
- Blocks use `{ }`
- Parameter lists use `( )`
- Generic type argument lists use `< >`
- Transition argument lists use `( )`

### Comments

Only single-line comments are supported:

```fal
// This is a comment
float x = 0.0;  // inline comment
```

---

## Types

### Primitive types

| Type | Description | Literals |
|------|-------------|---------|
| `int` | 64-bit signed integer | `0`, `-5`, `1000` |
| `float` | Double-precision float | `0.0`, `-3.14`, `1e-9` |
| `bool` | Boolean | `true`, `false` |
| `string` | String | `"hello"`, `""` |

### Struct types

Any declared struct name is a valid type (see [Structs](#structs) and [Generic Structs](#generic-structs)). Generic structs must be instantiated with concrete type arguments: `Box<int>`, `Pair<string, float>`.

Struct types from imported modules are referenced as `ModuleName::StructName` or just `StructName` if unambiguous.

### Nil

The literal `nil` represents a null/absent value and can appear in expressions.

---

## Autotuner Declaration

An autotuner is the top-level execution unit — a named state machine with typed inputs and outputs.

### Full syntax

```fal
autotuner Name (input_type input_name, ...) -> (output_type output_name, ...) {
    // 1. Variable declarations and initialisation (the "body" before start)
    output_name = default_value;
    type local_var = value;

    // 2. Entry state declaration
    start -> first_state_name;

    // 3. State definitions
    state first_state_name { ... }
    state another_state   { ... }
}
```

### Input and output parameters

Input parameters are **read-only** within the autotuner. Output parameters must be declared in the signature and initialised in the body before `start`.

```fal
autotuner Adder (int a, int b) -> (int sum) {
    sum = 0;               // initialise output
    start -> compute;

    state compute {
        sum = a + b;
        -> done;
    }

    state done { terminal; }
}
```

An autotuner with no inputs omits the input list entirely:

```fal
autotuner NoInput -> (string result) {
    result = "";
    start -> run;
    state run { result = "ok"; terminal; }
}
```

### Variable declarations in the autotuner body

Variables declared before `start` are **autotuner-scoped** — available in all states:

```fal
autotuner Counter -> (int count) {
    count = 0;
    int limit = 10;   // autotuner-scoped local variable
    start -> loop;

    state loop {
        count = count + 1;
        if (count < limit) { -> loop; }
        else               { -> done; }
    }

    state done { terminal; }
}
```

---

## Variable Declarations

Variables must be declared with an explicit type before use. A declaration may optionally include an initialiser.

```fal
int counter;              // declared, not yet initialised
int counter = 0;          // declared and initialised
float voltage = 0.0;
bool enabled = true;
string label = "sweep";
Box<int> b = Box.New(42); // generic struct instance
```

**Redeclaration is an error.** You cannot declare the same name twice in the same scope.

### Scoping rules

| Scope | Where declared | Visible in |
|-------|---------------|------------|
| Autotuner scope | Autotuner body (before `start`) + output params | All states |
| Input params | Autotuner signature `(...)` | All states — read-only |
| State-local | Inside a state body | That state only |
| State input params | State signature `state Name (...)` | That state — read-only |
| Struct field | Inside `struct` body | All routines of that struct — via bare name or `this.field` |

---

## States

A state is a named block of statements. Every autotuner must have at least one state, and every execution path must eventually reach a `terminal` statement.

### State without parameters

```fal
state my_state {
    // statements
    -> next_state;
}
```

### State with input parameters

States can receive typed values from the calling transition. These parameters are **read-only** inside the state.

```fal
state process (int value, bool flag) {
    int local = value + 1;
    if (flag == true) { -> done(local); }
    else              { -> error; }
}
```

Transitions to a parametrised state must pass matching expressions:

```fal
-> process(42, true);
-> process(computed_val, my_flag);
```

### Terminal state

`terminal;` ends the autotuner's execution. It can appear anywhere in a state body:

```fal
state done {
    count = count + 1;  // still runs
    terminal;
}
```

A minimal terminal state:

```fal
state done { terminal; }
```

---

## Statements

Inside a state (or routine) body, the following statements are valid:

### Variable declaration

```fal
int x;
float y = 3.14;
MyStruct obj = MyStruct.New(args);
Box<int> b = Box.New(99);
```

### Assignment

Assign to any **non-read-only** variable in scope:

```fal
counter = counter + 1;
voltage = 0.0;
label = "done";
```

Struct field assignment:

```fal
config.threshold = 0.8;
this.field = value;        // inside a struct routine
```

Multi-target assignment (assigns the same expression to multiple variables):

```fal
a, b = some_expr;
```

### Function / routine call as statement

```fal
some_routine(arg1, arg2);
Module::routine(arg);
obj.method(args);
```

### Transition

```fal
-> state_name;
-> state_name(expr1, expr2);
```

### Terminal

```fal
terminal;
```

### Conditional

See [Control Flow](#control-flow).

---

## Transitions

Transitions move execution from the current state to another state.

### Unconditional transition

```fal
-> next_state;
```

### Transition with arguments

Pass values to a state's input parameters:

```fal
-> process(result, true);
```

Arguments are positional and must match the target state's parameter list in type and order.

### Cross-module transition

Call a state in an autotuner loaded from an imported module:

```fal
-> ModuleName::autotuner_state;
-> ModuleName::autotuner_state(arg1, arg2);
```

---

## Control Flow

### if / elif / else

All branches must use braces `{ }`. The `elif` keyword (not `else if`) is used for chained conditions.

```fal
if (x < 0) {
    -> negative;
}
elif (x == 0) {
    -> zero;
}
elif (x < 100) {
    -> small;
}
else {
    -> large;
}
```

### Looping via self-transition

The language has no explicit loop construct. Looping is expressed as a self-transition:

```fal
autotuner IterationTest (int max_iterations) -> (int counter) {
    counter = 0;
    start -> loop;

    state loop {
        counter = counter + 1;
        if (counter < max_iterations) { -> loop; }
        else                          { -> done; }
    }

    state done { terminal; }
}
```

---

## Expressions

### Arithmetic operators

```fal
a + b
a - b
a * b
a / b
-a      // unary negation
```

### Comparison operators

```fal
a == b
a != b
a < b
a > b
a <= b
a >= b
```

### Logical operators

```fal
a && b    // AND
a || b    // OR
!a        // NOT
```

### Operator precedence (high to low)

1. `.` `[...]` `(...)` `::` — member access, index, call, scope
2. `!` unary `-`
3. `*` `/`
4. `+` `-`
5. `<` `>` `<=` `>=`
6. `==` `!=`
7. `&&`
8. `||`

### Function calls

```fal
add(a, b)                  // plain call
math_utils::add(a, b)      // module-qualified call
obj.Value()                // method call
Struct.Constructor(args)   // static-style constructor call
```

Call arguments are positional:

```fal
clamp(val, 0.0, 1.0)
```

### Member access and indexing

```fal
obj.field           // struct field read
obj[index]          // index expression
this.field          // inside a struct routine
```

### Literals

```fal
42        // int
3.14      // float
true      // bool
false     // bool
"hello"   // string
nil       // null value
```

---

## Routines

Routines are top-level (or struct-member) pure functions with typed inputs and outputs. They do not have states or transitions.

### Syntax

```fal
routine Name (type param1, type param2) -> (type out1, type out2) {
    // statements — same as a state body, minus transitions and terminal
    out1 = param1 + param2;
}
```

- Input params are read-only.
- Output params must be assigned before the routine returns.
- Routines may call other routines.

### Examples

**Single output:**

```fal
routine Adder (int a, int b) -> (int add) {
    add = a + b;
}
```

**No input params** (empty `()` or omitted):

```fal
routine get_version -> (string ver) {
    ver = "1.0.0";
}
```

**Multiple outputs:**

```fal
routine divmod (int a, int b) -> (int quotient, int remainder) {
    quotient  = a / b;
    remainder = a - (quotient * b);
}
```

**With conditionals:**

```fal
routine clamp (float val, float lo, float hi) -> (float out) {
    if (val < lo)      { out = lo; }
    elif (val > hi)    { out = hi; }
    else               { out = val; }
}
```

### Calling routines

Routines are called like functions. Their return values can be assigned to variables:

```fal
int result = Adder(3, 4);
float safe = clamp(input, 0.0, 1.0);
```

Multi-output routines: the language currently supports capturing the first output via assignment. Use struct returns or state parameters for multiple values.

### Routines from imported modules

Cross-module routines are called directly with their qualified name — no declaration is required:

```fal
int sum  = math_utils::add(a, b);
float ar = geometry::area_square(5.0);
```

---

## Structs

Structs define composite data types. They may have fields with optional defaults and member routines.

### Syntax

```fal
struct TypeName {
    type field_name;
    type field_with_default = value;

    routine MethodName (type param) -> (type out) {
        // body — can access fields via this.field or bare field name
    }
}
```

### Field declarations

Fields are declared exactly like variable declarations, with optional defaults:

```fal
struct SweepConfig {
    float start  = 0.0;
    float stop   = 1.0;
    float step   = 0.01;
    int   repeat = 1;
}
```

Fields without defaults must be initialised before use (typically in a `New` constructor routine).

### Member routines

Struct routines follow the same `routine` syntax. Inside a struct routine, fields can be accessed as bare names **or** via `this.field`. Both forms are equivalent:

```fal
struct Quantity {
    int value;

    routine New (int v) -> (Quantity q) {
        q.value = v;       // assign field on the returned instance
    }

    routine Get -> (int out) {
        out = this.value;  // read via this
    }

    routine Double -> (int out) {
        out = value * 2;   // bare field name also works
    }
}
```

### Constructing structs

The conventional pattern is a static-style factory routine:

```fal
Quantity q = Quantity.New(42);
```

`Quantity.New(42)` calls the `New` routine on the `Quantity` type, which returns a `Quantity` value.

### Calling methods

```fal
int v = q.Get();
int d = q.Double();
```

### Struct field assignment

From outside the struct:

```fal
q.value = 10;
config.step = 0.05;
```

From inside a struct routine:

```fal
this.value = 10;   // explicit
value = 10;        // bare name — identical
```

---

## Generic Structs

Structs can be parameterised with one or more type parameters. The type parameters are listed in angle brackets after the struct name in the definition, and replaced with concrete types at the use site.

### Definition syntax

```fal
struct Name <T> {
    T field;
    routine New (T v) -> (Name<T> out) { out.field = v; }
    routine Get -> (T out)             { out = field; }
}
```

Multiple type parameters:

```fal
struct Pair <K, V> {
    K key;
    V value;
    routine New (K k, V v) -> (Pair<K,V> p) {
        p.key   = k;
        p.value = v;
    }
}
```

### Instantiation

Replace the type parameters with concrete types at the use site:

```fal
Box<int>         b = Box.New(42);
Box<float>       f = Box.New(3.14);
Pair<string,int> p = Pair.New("x", 10);
```

The interpreter resolves the concrete type at runtime (monomorphisation-on-demand). Each distinct instantiation is a separate type at runtime.

### Using generic types as parameters and return types

Generic struct types can appear anywhere a type is valid — in routine signatures, state parameters, autotuner inputs/outputs, and field declarations:

```fal
struct Accumulator <T> {
    T total;

    routine New (T init) -> (Accumulator<T> acc) {
        acc.total = init;
    }

    routine Add (T delta) -> (T new_total) {
        total = total + delta;   // bare field name inside routine
        new_total = total;
    }

    routine Value -> (T v) {
        v = this.total;
    }
}

autotuner GenericMath (int start_val, int add_val) -> (int result) {
    result = 0;
    start -> run;
    state run {
        Accumulator<int> acc = Accumulator.New(start_val);
        result = acc.Add(add_val);
        terminal;
    }
}
```

### Arity checking

Passing the wrong number of type arguments is a parse-time error:

```fal
// WRONG — Box expects 1 type argument, not 2
Box<int, float> b = Box.New(x);
// ERROR: Struct 'Box' expects 1 type argument(s) but got 2
```

### Generic structs in struct fields

A generic struct may have fields whose type is one of its own type parameters, or another concrete/generic struct:

```fal
struct Wrapper <T> {
    T inner;
    Box<T> boxed;

    routine New (T v) -> (Wrapper<T> w) {
        w.inner = v;
        w.boxed = Box.New(v);
    }
}
```

---

## Imports and Modules

### Single import

```fal
import "path/to/file.fal";
```

### Multi-path import

```fal
import (
    "./Adder.fal"
    "./Multiplier.fal"
    "./types/Config.fal"
)
```

Paths are relative to the importing file. Imports must appear at the very top of the file, before any declarations.

### Module names

When a file is imported, its filename (without `.fal`) becomes its module name. All routines, structs, and autotuners from that file are accessible via `ModuleName::symbol`:

```fal
import "geometry.fal";
// now geometry::area_square is accessible

import "math_utils.fal";
// now math_utils::add is accessible
```

### Qualified calls

Cross-module routines are called directly with their qualified name; no prior declaration is required:

```fal
float a = geometry::area_square(5.0);
int   s = math_utils::add(x, y);
```

Transitive imports work — if `geometry.fal` imports `math_utils.fal`, then `math_utils::get_version()` is reachable from the file that imports geometry.

### Importing structs

Structs from an imported file are usable as types once imported:

```fal
import "./Quantity.fal";
import "./Connection.fal";

autotuner MultipleStruct (int a, int b) -> (int sum, string name) {
    sum  = 0;
    name = "";
    start -> calculate;

    state calculate {
        Quantity   q = Quantity::quantity.New(a);
        Connection c = Connection::connection.New("test");
        sum  = q.Value() + b;
        name = c.Name();
        -> done;
    }

    state done { terminal; }
}
```

---

## FFI (Foreign Function Interface)

`ffimport` binds a C++ source file to `.fal` symbols. The engine compiles the wrapper on first use via the C ABI and caches the resulting `.so` — **no manual CMake step is required**.

### Syntax

```fal
ffimport "wrapper.cpp"
    ("-I/path/to/headers" "-I/other/path")
    ("-lmylibrary" "-L/path/to/lib")
```

- First `( )` — compiler `-I` include flags (can be empty `()`)
- Second `( )` — linker flags and `-l` libraries (can be empty `()`)

### Writing the wrapper

All wrapper functions must be `extern "C"` and follow the Falcon C ABI:

```cpp
// hardware_wrapper.cpp
#include <falcon-typing/falcon_ffi.h>
#include <falcon-typing/FFIHelpers.hpp>

extern "C" void measure_iv(
    const FalconParamEntry* in,  int32_t in_count,
    FalconResultSlot*       out, int32_t* out_count)
{
    auto params    = falcon::typing::ffi::engine::unpack_params(in, in_count);
    double voltage = std::get<double>(params.at("voltage"));

    double current = MyHardware::measure(voltage);
    falcon::typing::ffi::engine::set_result(out, out_count, 0, current);
}
```

The engine compiles the `.cpp` automatically when the `.fal` file is loaded. You do **not** need to write a CMakeLists.txt, run `cmake`, or link manually.

Struct methods use the `STRUCTTypeName` prefix convention:

```cpp
extern "C" void STRUCTDeviceNew(
    const FalconParamEntry* in,  int32_t in_count,
    FalconResultSlot*       out, int32_t* out_count)
{ /* constructor body */ }
```

### Full ffimport example

```fal
ffimport "hardware_wrapper.cpp"
    ("-I/opt/mydevice/include")
    ("-lmydevice")

autotuner HardwareSweep (float start_v, float stop_v) -> (float peak) {
    peak = 0.0;
    float current_v = start_v;
    start -> sweep;

    state sweep {
        float measured = measure_iv(current_v);
        if (measured > peak) { peak = measured; }
        current_v = current_v + 0.01;
        if (current_v < stop_v) { -> sweep; }
        else                    { -> done;  }
    }

    state done { terminal; }
}
```

---

## Complete Examples

### Example 1: Simple sequential autotuner

```fal
autotuner SequentialTest -> (int step) {
    step = 0;
    start -> state1;

    state state1 { step = 1; -> state2; }
    state state2 { step = 2; -> state3; }
    state state3 { step = 3; -> done;   }
    state done   { terminal; }
}
```

---

### Example 2: Iteration (looping via self-transition)

```fal
autotuner IterationTest (int max_iterations) -> (int counter) {
    counter = 0;
    start -> loop;

    state loop {
        counter = counter + 1;
        if (counter < max_iterations) { -> loop; }
        else                          { -> done; }
    }

    state done { terminal; }
}
```

---

### Example 3: Conditional chain with elif

```fal
autotuner ConditionalChain (int value) -> (string category) {
    category = "";
    start -> classify;

    state classify {
        if (value < 0)    { -> negative; }
        elif (value == 0) { -> zero;     }
        elif (value < 10) { -> small;    }
        elif (value < 100){ -> medium;   }
        else              { -> large;    }
    }

    state negative { category = "negative"; terminal; }
    state zero     { category = "zero";     terminal; }
    state small    { category = "small";    terminal; }
    state medium   { category = "medium";   terminal; }
    state large    { category = "large";    terminal; }
}
```

---

### Example 4: Sweep with float parameters

```fal
autotuner SimpleSweep (float begin, float end, float step) -> (int count, float final_value) {
    float current = 0.0;
    count = 0;
    final_value = 0.0;
    start -> init;

    state init {
        current = begin;
        -> sweep;
    }

    state sweep {
        count = count + 1;
        final_value = current;
        current = current + step;
        if (current <= end) { -> sweep; }
        else                { -> done;  }
    }

    state done { terminal; }
}
```

---

### Example 5: State input parameters (passing values between states)

```fal
routine Adder      (int a, int b) -> (int add)  { add  = a + b; }
routine Multiplier (int a, int b) -> (int mult) { mult = a * b; }

autotuner ConditionalNest (int a, int b) -> (int out) {
    out = 0;
    start -> init;

    state init {
        int add = Adder(a, b);
        -> multiplication(add);
    }

    state multiplication (int c) {
        int result = Multiplier(c, b);
        -> done(result);
    }

    state done (int out_inside) {
        out = out_inside;
        terminal;
    }
}
```

---

### Example 6: Routines and module imports

```fal
// Adder.fal
routine adder (int a, int b) -> (int add) { add = a + b; }

// Multiplier.fal
routine multiplier (int a, int b) -> (int mult) { mult = a * b; }

// main.fal
import (
    "./Adder.fal"
    "./Multiplier.fal"
)

autotuner ConditionalNest (int a, int b) -> (int out) {
    out = 0;
    start -> init;

    state init {
        int add = Adder::adder(a, b);       // qualified cross-module call
        -> multiplication(add);
    }

    state multiplication (int c) {
        int result = Multiplier::multiplier(c, b);
        -> done(result);
    }

    state done (int out_inside) {
        out = out_inside;
        terminal;
    }
}
```

**Note:** No `uses` clause is required. Cross-module calls work directly via the `Module::symbol` qualified syntax.

---

### Example 7: Non-generic struct with methods

```fal
struct Quantity {
    int a_;

    routine New (int a) -> (Quantity q) {
        q.a_ = a;
    }

    routine Value -> (int value) {
        value = a_;        // bare field name (no this needed)
    }
}

autotuner QuantityStruct (int a, int b) -> (int sum) {
    sum = 0;
    start -> calculate;

    state calculate {
        Quantity q = Quantity.New(a);
        sum = q.Value() + b;
        -> done;
    }

    state done { terminal; }
}
```

---

### Example 8: Generic struct — Accumulator

```fal
struct Accumulator <T> {
    T total;

    routine New (T init) -> (Accumulator<T> acc) {
        acc.total = init;
    }

    routine Add (T delta) -> (T new_total) {
        total     = total + delta;   // bare field — updates the struct's own state
        new_total = total;
    }

    routine Value -> (T v) {
        v = this.total;
    }
}

autotuner GenericMath (int start_val, int add_val) -> (int result) {
    result = 0;
    start -> run;
    state run {
        Accumulator<int> acc = Accumulator.New(start_val);
        result = acc.Add(add_val);
        terminal;
    }
}
```

---

### Example 9: Generic struct — two type parameters

```fal
struct Pair <K, V> {
    K key;
    V value;

    routine New (K k, V v) -> (Pair<K,V> p) {
        p.key   = k;
        p.value = v;
    }

    routine GetKey   -> (K out) { out = key; }
    routine GetValue -> (V out) { out = value; }
}

autotuner PairDemo (string label, int data) -> (string k, int v) {
    k = ""; v = 0;
    start -> run;
    state run {
        Pair<string,int> p = Pair.New(label, data);
        k = p.GetKey();
        v = p.GetValue();
        terminal;
    }
}
```

---

### Example 10: Cross-module namespacing and transitive imports

```fal
// math_utils.fal
routine add (int a, int b) -> (int sum) { sum = a + b; }
routine get_version -> (string ver)     { ver = "1.0.0"; }

// geometry.fal
import "math_utils.fal";
routine area_square (float side) -> (float res) { res = side * side; }

// main.fal
import "geometry.fal";

autotuner NamespacingTest (int a, int b) -> (int sum, float area, string version) {
    start -> run;
    state run {
        sum     = math_utils::add(a, b);           // transitive: geometry imports math_utils
        area    = geometry::area_square(5.0);
        version = math_utils::get_version();
        terminal;
    }
}
```

---

## Common Mistakes

### 1. Using the removed `uses` clause

`uses` is no longer a keyword and will cause a parse error.

```fal
// WRONG — uses is not a valid keyword
autotuner Bad (int x) -> (int y) {
    uses Adder::adder;
    ...
}

// CORRECT — call cross-module routines directly by qualified name
autotuner Good (int x) -> (int y) {
    y = 0;
    start -> run;
    state run {
        y = Adder::adder(x, 1);
        terminal;
    }
}
```

### 2. Using old `requires:` syntax

```fal
// WRONG — requires: was never part of the language
autotuner Bad {
    requires: [OtherAutotuner];
    ...
}

// CORRECT — no declaration needed; call cross-module symbols directly
autotuner Good (int x) -> (int y) {
    y = 0;
    start -> run;
    state run { y = x + 1; terminal; }
}
```

### 3. Using old `params {}` / `temp {}` block syntax

```fal
// WRONG — there is no params {} block
autotuner Bad {
    params { int x = 0; }
}

// CORRECT — declare outputs in the signature, vars inline before start
autotuner Good -> (int x) {
    x = 0;
    start -> run;
    state run { x = 1; terminal; }
}
```

### 4. Wrong number of generic type arguments

```fal
// WRONG — Box expects 1 type argument
Box<int, float> b = Box.New(x);
// ERROR: Struct 'Box' expects 1 type argument(s) but got 2

// CORRECT
Box<int> b = Box.New(x);
```

### 5. Using bracket `[var]` transition syntax

```fal
// WRONG — brackets are not used for transitions
-> next_state[voltage];

// CORRECT — pass values as positional arguments
-> next_state(voltage);
```

And the receiving state must declare the parameter:

```fal
state next_state (float voltage) {
    terminal;
}
```

### 6. Using `else if` instead of `elif`

```fal
// WRONG
if (x < 0) { -> neg; }
else if (x == 0) { -> zero; }

// CORRECT
if (x < 0) { -> neg; }
elif (x == 0) { -> zero; }
```

### 7. Omitting braces from if/else

Every `if`, `elif`, and `else` branch **must** use braces:

```fal
// WRONG
if (x < 0) -> neg;

// CORRECT
if (x < 0) { -> neg; }
```

### 8. Assigning to a read-only input parameter

```fal
autotuner Bad (int input) -> (int out) {
    input = 5;  // ERROR: cannot assign to input parameter
    ...
}
```

### 9. Using a variable before it is declared

```fal
state check {
    result = 1;        // ERROR: 'result' not declared
    int result = 0;    // declaration must come first
}
```

### 10. Using `measurement:` keyword

There is no `measurement:` keyword. Call functions directly as statements or in expressions:

```fal
// WRONG
measurement: do_work(arg);

// CORRECT
do_work(arg);
int r = do_work(arg);
```

---

## Grammar Summary

```ebnf
program          := import_list program_item*

import_list      := import_stmt*

import_stmt      := "import" STRING ";"
                  | "import" "(" STRING+ ")"

ffimport_decl    := "ffimport" STRING
                    "(" STRING* ")"
                    "(" STRING* ")"

program_item     := struct_decl
                  | routine_decl
                  | autotuner_decl
                  | ffimport_decl

struct_decl      := "struct" IDENTIFIER "{" struct_field* routine_decl* "}"
                  | "struct" IDENTIFIER "<" generic_param_list ">" "{" struct_field* routine_decl* "}"

generic_param_list := IDENTIFIER ("," IDENTIFIER)*

struct_field     := type_spec IDENTIFIER ";"
                  | type_spec IDENTIFIER "=" expr ";"

routine_decl     := "routine" IDENTIFIER input_params "->" output_params routine_body

routine_body     := "{" stmt* "}"
                  | %empty            // empty body = FFI stub

autotuner_decl   := "autotuner" IDENTIFIER input_params? "->" output_params "{"
                      stmt*           // variable decls and initialisations
                      entry_state
                      state_decl+
                    "}"

input_params     := "(" param_list ")"  | "()" | %empty
output_params    := "(" param_list ")"  | "()"

param_list       := param_decl ("," param_decl)*

param_decl       := type_spec IDENTIFIER
                  | type_spec IDENTIFIER "=" expr

entry_state      := "start" "->" IDENTIFIER ";"

state_decl       := "state" IDENTIFIER state_params "{" stmt* "}"

state_params     := "(" param_list ")"  | "()" | %empty

stmt             := var_decl_stmt
                  | assign_target_list "=" expr ";"
                  | IDENTIFIER "." IDENTIFIER "=" expr ";"
                  | "this" "." IDENTIFIER "=" expr ";"
                  | "->" IDENTIFIER ";"
                  | "->" IDENTIFIER "(" expr_list ")" ";"
                  | "terminal" ";"
                  | "if" "(" expr ")" "{" stmt* "}" elif_chain
                  | expr ";"

elif_chain       := %empty
                  | "else" "{" stmt* "}"
                  | "elif" "(" expr ")" "{" stmt* "}" elif_chain

var_decl_stmt    := type_spec IDENTIFIER ";"
                  | type_spec IDENTIFIER "=" expr ";"

type_spec        := "int" | "float" | "bool" | "string"
                  | qualified_name
                  | qualified_name "<" type_arg_list ">"

type_arg_list    := type_spec ("," type_spec)*

qualified_name   := IDENTIFIER "::" IDENTIFIER  |  IDENTIFIER

expr             := literal
                  | IDENTIFIER
                  | qualified_name
                  | "this"
                  | "nil"
                  | expr "+" expr  | expr "-" expr
                  | expr "*" expr  | expr "/" expr
                  | expr "==" expr | expr "!=" expr
                  | expr "<" expr  | expr ">" expr
                  | expr "<=" expr | expr ">=" expr
                  | expr "&&" expr | expr "||" expr
                  | "!" expr  |  "-" expr
                  | "(" expr ")"
                  | expr "." IDENTIFIER                       // member access
                  | expr "." IDENTIFIER "(" call_arg_list ")" // method call
                  | expr "[" expr "]"                         // index
                  | IDENTIFIER "(" call_arg_list ")"          // function call
                  | qualified_name "(" call_arg_list ")"      // qualified call

call_arg         := expr

literal          := INTEGER | DOUBLE | STRING | "true" | "false" | "nil"
```

---

## See Also

- [CLI.md](CLI.md) — `falcon-run` and `falcon-test` command-line reference
- [LSP.md](LSP.md) — Editor integration
- [PACKAGE_MANAGER.md](PACKAGE_MANAGER.md) — Package management
- [../libs/testing/README.md](../libs/testing/README.md) — Testing library reference
