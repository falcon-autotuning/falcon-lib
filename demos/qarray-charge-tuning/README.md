# QArray Charge Tuning Demo & Tests

A modern demo for QArray charge tuning in FAlCon, using the device characteristics database for global variables (libpqxx backend).

This demo only currently runs on Linux, it may support WSL.

The main two autotuners responsible for this measurement are the ChargeConfigurationTuner and the BlipStateStepper.
The ChargeConfigurationTuner runs the high level autotuning of the charge system, and the StateStepper handles the finer details of how to actually accomplish crossing a charge boundary.

We recommend running this demo using the falcon-test CLI. We have setup a psuedotest that sets up the environment for the autotuner and generates a local Charge Stability Diagram of the local area.

## Setup

### 1. Establish the database

Ensure PostgreSQL is running and create the Falcon test database:

```bash
export TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@127.0.0.1:5432/falcon_test"
```

### 2. Run the demo

The log-level lets users customize the amount of logging they want to see when running the CLI.

```bash
# From the demos/qarray-charge-tuning
make docker-up
# this runs the test
falcon-test ./tests/run_tests.fal --log-level info
```

### 3. Feel free to customize the measurements

As a follow-up to the description in the Arxiv preprint, this demo goes more in detail into the specifics of FAlCon.
One of the most important details with autotuning systems in general is **Variable scoping**.
Algorithm writers need to maintain exact management of these variables as they transition in and out of scope.
The simplest example of something like this

```fal
autotuner Blip -> (Error err) {
int autotuner_scope_variable;
start -> init;
state init {
  int state_scope_variable;
  terminal;
}
}
```

where we have explicitly labelled the scope of the variables.
When inside of a state like the **init** state, you have access to both the autotuner_scope_variable and the state_scope_variable.
However the autotuner does not know about the state_scope_variable.

You could imagine more complex scenarios where you might want to carry variables over through the scope of the outermost autotuner and inject themselves directly when needed.
A good example of this might be sweep resolution.
Piping this through all the autotuner layers above is inefficient.
Instead we can make it appear in thin-air.

This is what we use the PostgreSQL database for.
There is a CLI that users can use to set this up when running this in real scenarios but for now we construct them in the test.
