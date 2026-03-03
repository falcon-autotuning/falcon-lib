# Falcon DSL Language Reference

## Table of Contents

1. [Overview](#overview)
2. [File Structure](#file-structure)
3. [Syntax Rules](#syntax-rules)
4. [Autotuner Declaration](#autotuner-declaration)
5. [Parameters](#parameters)
6. [States](#states)
7. [Transitions](#transitions)
8. [Variable Scoping](#variable-scoping)
9. [Measurements](#measurements)
10. [Comments](#comments)
11. [Complete Examples](#complete-examples)

---

## Overview

The Falcon DSL Language (.fal) is a domain-specific language for defining state machines used in quantum device autotuning. It provides:

- **Declarative state machine definition**
- **Type-safe parameter passing**
- **Explicit variable scoping (persistent vs temporary)**
- **Cross-autotuner state transitions**
- **Clean separation between control flow and measurement logic**

---

## File Structure

A `.fal` file consists of one or more autotuner declarations:

```fal
// autotuner 1
autotuner MyFirstAutotuner {
  // ... definition ...
}

// autotuner 2
autotuner MySecondAutotuner {
  // ... definition ...
}
```

---

## Syntax Rules

### Case Sensitivity

- Keywords are lowercase: autotuner, state, params, temp, if, else, terminal
- Identifiers (names) are case-sensitive: myState ≠ MyState

### Naming Conventions

- Autotuner names: PascalCase (e.g., VoltageSweep, CalibrationRoutine)
- State names: snake_case (e.g., initialize, sweep_loop, error_recovery)
- Parameter names: snake_case (e.g., min_voltage, iteration_count)
- Function names: snake_case (e.g., measure_voltage, handle_error)

### Whitespace

- Whitespace (spaces, tabs, newlines) is generally ignored
- Use indentation for readability (2 or 4 spaces recommended)

### Terminators

- Statements end with semicolon ;
- Blocks use braces { }

---

## Autotuner Declaration

```fal
autotuner AutotunerName {
  requires: [Dependency1, Dependency2];
  
  params {
    // persistent parameters
  }
  
  start -> entry_state;
  
  // state definitions
}
```

### Componenta

==requires== (optional)

Lists other autotuners this one depends on (for cross-autotuner transitions).

```fal
requires: [CalibrationAutotuner, ErrorHandler];
```

==start==

Specifies the entry state (required).

```fal
start -> initialization;
```

---

## Parameters

### Persistent Parameters

Defined in params block. These exist throughout the autotuner's execution.

```fal
params {
  float min_voltage = 0.0;      // Floating point with default
  int max_iterations = 100;     // Integer
  bool debug_mode = false;      // Boolean
  string device_id = "dev0";    // String
}
```

### Supported Types

| Type   | Description             | Example Values         |
|--------|-------------------------|-----------------------|
| ==int==    | 64-bit signed integer   | ==0==, ==-5==, ==1000==           |
| ==float==  | Double precision float  | ==0.0==, ==-3.14==, ==1e-6==      |
| ==bool==   | Boolean                 | ==true==, ==false==           |
| ==string== | String literal          | =="hello"==, =="device_0"==   |

### Type Declarations

Parameters must specify their type:

```fal
// Correct
float voltage = 1.0;
int counter = 0;

// Incorrect
voltage = 1.0;      // Missing type
auto x = 5;         // 'auto' not supported
```

---

## States

### Basic State

```fal
state state_name {
  measurement: function_name(arg1, arg2);
  
  if (condition) -> next_state;
  else -> alternative_state;
}
```

### Terminal State

A state with no outgoing transitions (end of execution):

```fal
state complete {
  terminal;
}
```

### State Components

1. Temporary Parameters

  Variables that only exist within this state:

  ```fal
  state measure {
    temp {
      float measured_value;
      bool measurement_valid;
      string status_message;
    }
    
    measurement: take_measurement();
    // Returns: measured_value, measurement_valid, status_message
    
    if (measurement_valid == true) -> process;
    else -> error;
  }
  ```

1. Received Parameters

  Parameters transferred from previous state:

  ```
  ```fal
  state process {
    params {
      float input_value;    // Received from previous state
      string operation;     // Received from previous state
    }
    
    measurement: process_data(input_value, operation);
    
    -> done;
  }
  ```

1. Measurement

  Calls a C++ function to perform work:

  ```fal
  measurement: function_name(param1, param2);
  ```

  The function must:

- Be defined in the autotuner's namespace in C++
- Take ==const ParameterMap&== as argument
- Return ==ParameterMap== with results

---

## Transitions

### Unconditional Transition

```fal
-> next_state;
```

### Conditional Transition

```fal
if (condition) -> state1;
else if (other_condition) -> state2;
else -> default_state;
```

### Conditions

Comparison Operators

```fal
voltage == 1.5          // Equal
voltage != 0.0          // Not equal
counter < 10            // Less than
counter > 0             // Greater than
voltage <= 1.0          // Less than or equal
voltage >= 0.5          // Greater than or equal
```

Logical Operators

```fal
voltage > 0.5 && voltage < 1.5     // AND
success == true || retry == true    // OR
!(error_occurred)                   // NOT
```

Boolean Literals

```fal
if (success == true) -> continue;
if (enabled == false) -> skip;
```

### Cross-Autotuner Transitions

Reference states in other autotuners:

```fal
if (needs_calibration) -> CalibrationAutotuner::start;
```

Format: ==AutotunerName::state_name==

---

## Variable Scoping

### Persistent Parameters

Defined in autotuner's ==params== block. Available in all states.

```fal
params {
  float voltage = 0.0;
}

state state1 {
  // voltage is accessible here
  measurement: func1();
  voltage = voltage + 0.1;  // Modify persistent param
  -> state2;
}

state state2 {
  // voltage still accessible with updated value
  measurement: func2();
  -> done;
}
```

### Temporary Parameters

Defined in state's ==temp== block. Only exist within that state.

```fal
state measure {
  temp {
    float result;    // Only exists in 'measure' state
  }
  
  measurement: get_result();
  // Sets: result
  
  -> next[result];  // Transfer to next state
}

state next {
  params {
    float result;    // Receives 'result' from previous
  }
  
  measurement: use_result(result);
  -> done;
}
```

### Parameter Transfer

Transfer variables between states using brackets ==[]==:
Auto-transfer (same name)

```fal
state state1 {
  temp {
    float voltage;
  }
  
  measurement: measure();
  
  -> state2[voltage];  // Transfer as 'voltage'
}

state state2 {
  params {
    float voltage;      // Auto-matched by name
  }
  
  measurement: process(voltage);
  -> done;
}
```

Explicit mapping

```fal
state state1 {
  temp {
    float measured_v;
  }
  
  measurement: measure();
  
  -> state2[measured_v: input_voltage];  // Rename during transfer
}

state state2 {
  params {
    float input_voltage;    // Receives as 'input_voltage'
  }
  
  measurement: process(input_voltage);
  -> done;
}
```

Multiple transfers

```fal
-> next_state[var1, var2: mapped_var2, var3];
```

Equivalent to:

- ==var1== → ==var1== (same name)
- ==var2== → ==mapped_var2== (renamed)
- ==var3== → ==var3== (same name)

---

## Measurements

### Declaration in .fal

```fal
state my_state {
  measurement: my_function(arg1, arg2);
  -> next;
}
```

### Implementation in C++

Must be in the autotuner's namespace:

```C++
namespace MyAutotuner {

using namespace falcon::dsl;

ParameterMap my_function(const ParameterMap& params) {
    ParameterMap result;
    
    // Get input parameters
    float arg1 = params.get<float>("arg1");
    float arg2 = params.get<float>("arg2");
    
    // Perform measurement/computation
    float output = arg1 + arg2;
    
    // Set output parameters
    result.set("output", output);
    result.set("success", true);
    
    return result;
}

} // namespace MyAutotuner
```

### Return Values

Measurements return ==ParameterMap== containing:

- Temporary variables defined in state's ==temp== block
- Any additional data needed for transitions

```fal
state measure {
  temp {
    float current;
    float voltage;
    bool valid;
  }
  
  measurement: measure_iv();
  // Must return: current, voltage, valid
  
  if (valid == true) -> process;
  else -> error;
}
```

---

## Comments

### Single-line comments

```fal
// This is a comment
state my_state {  // Comment after code
  -> next;
}
```

### Block comments

Not supported. Use multiple single-line comments:

```fal
// This is a multi-line
// comment using
// multiple single-line comments
```

---

## Complete Examples

### Example 1: Simple Linear Flow

```fal
autotuner SimpleFlow {
  requires: [];
  
  params {
    int counter = 0;
  }
  
  start -> init;
  
  state init {
    temp {
      bool success;
    }
    
    measurement: initialize();
    
    if (success == true) -> process;
    else -> error;
  }
  
  state process {
    measurement: do_work();
    
    counter = counter + 1;
    
    if (counter < 10) -> process;
    else -> done;
  }
  
  state done {
    terminal;
  }
  
  state error {
    terminal;
  }
}
```

### Example 2: Parameter Transfer

```fal
autotuner ParameterTransfer {
  requires: [];
  
  params {
    float threshold = 1.0;
  }
  
  start -> measure;
  
  state measure {
    temp {
      float value;
      bool valid;
    }
    
    measurement: get_measurement();
    
    if (valid == true) -> check[value];
    else -> error;
  }
  
  state check {
    params {
      float value;      // Received from 'measure'
    }
    
    measurement: validate_measurement(value);
    
    if (value > threshold) -> high_handler[value: measured];
    else -> low_handler[value: measured];
  }
  
  state high_handler {
    params {
      float measured;   // Received as 'measured' (was 'value')
    }
    
    measurement: handle_high(measured);
    -> done;
  }
  
  state low_handler {
    params {
      float measured;
    }
    
    measurement: handle_low(measured);
    -> done;
  }
  
  state done {
    terminal;
  }
  
  state error {
    terminal;
  }
}
```

### Example 3: Cross-Autotuner Transitions

```fal
autotuner MainAutotuner {
  requires: [RecoveryAutotuner];
  
  params {
    int attempts = 0;
  }
  
  start -> try_operation;
  
  state try_operation {
    temp {
      bool success;
    }
    
    measurement: attempt_operation();
    
    if (success == true) -> complete;
    else {
      attempts = attempts + 1;
      if (attempts < 3) -> try_operation;
      else -> RecoveryAutotuner::recover[attempts: failed_attempts];
    }
  }
  
  state complete {
    terminal;
  }
}

autotuner RecoveryAutotuner {
  requires: [];
  
  params {
    int failed_attempts = 0;
  }
  
  start -> recover;
  
  state recover {
    temp {
      bool recovered;
    }
    
    measurement: attempt_recovery(failed_attempts);
    
    if (recovered == true) -> MainAutotuner::complete;
    else -> failed;
  }
  
  state failed {
    terminal;
  }
}
```

---

## Formatting Best Practices

### Indentation

Use 2 or 4 spaces (consistent throughout file):

```fal
autotuner Example {
  params {
    int x = 0;
  }
  
  start -> init;
  
  state init {
    temp {
      bool success;
    }
    
    measurement: initialize();
    
    if (success == true) -> done;
    else -> error;
  }
  
  state done {
    terminal;
  }
  
  state error {
    terminal;
  }
}
```

### Line Length

Keep lines under 100 characters for readability.

### Blank Lines

- One blank line between states
- One blank line between major sections
- No blank line between state name and opening brace

### Alignment

Align similar elements for readability:

```fal
params {
  float min_voltage   = 0.0;
  float max_voltage   = 1.0;
  float step          = 0.1;
  int   max_iter      = 100;
}
```

---

## Common Mistakes

1. Missing Semicolons

```fal
// Wrong
params {
  int x = 0
}

// Correct
params {
  int x = 0;
}
```

1. Undefined Variables in Conditions

```fal
state check {
  // 'result' not defined in temp or params
  if (result > 10) -> next;  // ERROR
}
```

1. Type Mismatches

```fal
params {
  int counter = 0;
}

state check {
  // Comparing int to float
  if (counter > 10.5) -> next;  // Works but may be unintended
}
```

1. Transferring Undefined Variables

```fal
state state1 {
  temp {
    float x;
  }
  
  measurement: func();
  
  -> state2[y];  // ERROR: 'y' not defined
}
```

1. Missing Terminal States

Every execution path must end at a ==terminal== state.

---

## Grammar Summary

```EBNF
program         := autotuner+

autotuner       := "autotuner" IDENTIFIER "{"
                   requires? params? start state+
                   "}"

requires        := "requires" ":" "[" identifier_list "]" ";"

params          := "params" "{" param_decl* "}"

param_decl      := type IDENTIFIER "=" literal ";"

start           := "start" "->" IDENTIFIER ";"

state           := "state" IDENTIFIER "{"
                   state_params? temp_params? measurement? transitions? terminal?
                   "}"

state_params    := "params" "{" param_decl* "}"

temp_params     := "temp" "{" param_decl* "}"

measurement     := "measurement" ":" function_call ";"

function_call   := IDENTIFIER "(" argument_list? ")"

transitions     := transition+

transition      := ("if" "(" condition ")" | "else")
                   assignment* "->" target ";"
                 | "->" target ";"

target          := IDENTIFIER ("::" IDENTIFIER)? transfer?

transfer        := "[" transfer_list "]"

transfer_list   := transfer_item ("," transfer_item)*

transfer_item   := IDENTIFIER (":" IDENTIFIER)?

assignment      := IDENTIFIER "=" expression ";"

terminal        := "terminal" ";"

condition       := expression

expression      := ... (standard boolean/arithmetic expressions)

type            := "int" | "float" | "bool" | "string"

literal         := INT_LITERAL | FLOAT_LITERAL | BOOL_LITERAL | STRING_LITERAL
```

---

## See Also

- Tutorial - Step-by-step guide
- Examples - Real-world examples
- C++ API Reference - Implementing measurements
