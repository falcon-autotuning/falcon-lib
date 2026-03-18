# QArray Charge Tuning Demo

## Overview

This demo demonstrates QArray charge tuning using FAlCon, leveraging a device characteristics database for robust global variable management.  
**Supported Platforms:** Linux (native); Windows Subsystem for Linux (WSL) may be compatible.

## Key Components

- **ChargeConfigurationTuner**: Manages high-level charge system autotuning.
- **BlipStateStepper**: Provides fine-grained control for crossing charge boundaries.

## Getting Started

### Prerequisites

- Linux or WSL
- Docker
- FAlCon libraries installed at `/opt/falcon/lib`

### Installing Docker

Here are installation instructions for a few common linux distributions. For other distributions, please refer to the official [Docker documentation](https://docs.docker.com/desktop/setup/install/linux/).
**Ubuntu/Debian:**

```sh
sudo apt update
sudo apt install docker.io docker-compose
```

**Fedora:**

```sh
sudo dnf install docker docker-compose
```

**Arch Linux:**

```sh
sudo pacman -S docker docker-compose
```

Depending on your system you may need to log out and log back in to get Docker fully setup on your system.

### Environment Setup

Set the required environment variables before running the demo:

```bash
export PATH="/opt/falcon/lib:$PATH"
export LD_LIBRARY_PATH="/opt/falcon/lib:$LD_LIBRARY_PATH"
export PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig:$PKG_CONFIG_PATH"
```

### Database Setup

Ensure PostgreSQL is running and configure the Falcon test database:

```bash
export TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@127.0.0.1:5432/falcon_test"
```

### Running the Demo

Clone the repository and navigate to the demo directory:

```bash
cd demos/qarray-charge-tuning
make docker-up
falcon-test ./tests/run_tests.fal --log-level info
```

- Adjust `--log-level` for desired verbosity (`info`, `debug`, etc.).

## Expected Output

The Device Under Test (DUT) is a double quantum dot system. The autotuner is tasked with tuning to the (1,2) charge configuration. The device includes a charge sensor, and the differential signal is visualized.

![Charge Stability Diagram](images/stability_diagram.png)

The autotuner adjusts plunger gate voltages to locate the (1,2) charge configuration, using the BlipStateStepper to navigate the charge stability diagram and cross charge boundaries as needed.

- **P1 Sweep:** Initial search in the P1 direction.
  ![vP1 sweep](images/analyze_blips_signal_1.png)
- **P2 Sweep:** Subsequent searches in the P2 direction to reach the target configuration.
  ![vP2 sweep](images/analyze_blips_signal_2.png)
  ![vP3 sweep](images/analyze_blips_signal_3.png)

## Explaining the Organization

This section will detail explaining some of the code details, and how this demo is implemented.

### Variable Scoping

Variable scoping is critical for charge stability diagram measurements. Variables may be defined at different levels:

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

- Within the `init` state, both `autotuner_scope_variable` and `state_scope_variable` are accessible.
- The autotuner itself does not have access to `state_scope_variable`.

For advanced scenarios (e.g., sweep resolution), variables can be injected at higher scopes using the PostgreSQL database.  
While a CLI exists for real deployments, this demo constructs them directly in the test.

### The Test File Itself

This test file is written using the [testing framework](https://github.com/falcon-autotuning/falcon-lib/tree/main/libs/testing) where the details of how to use the framework are explained in the library.
We have included only a single test in this suite, feel free to add more or customize our directly.
The test is called test_Charge1AndCharge2 and it attempts to tune the DUT to the (1,2) charge configuration.

The test begins by first restarting the attached [QArray](https://github.com/b-vanstraaten/QArray) simulation.
This QArray simulation is bound into the FAlCon DSL through the FFI that we designed.
As a result we have bound the simulation into an "effective device" in our C++ [package](https://github.com/falcon-autotuning/falcon-lib/tree/main/qarrayDevice).
This effective device manages its own voltage settings like a standard QD device in a laboratory setting.
In addition it can be configured to be either electron or holes via its constructor, and the finer details of configuring the device can be specified in the configuration file for the simulation.

Here is the yaml file for convenience

```yaml
gate_names:
  - P1
  - P2
  - B1
  - S1
sensor_config:
  S1: true
capacitances:
  Cgd:
    - [5.0, 0.5, 0.0, 0.0]
    - [0.5, 5.0, 0.0, 0.0]
  Cgs:
    - [0.01, 0.01, 0.01, 1.0]
```

This file specifies specifically the gates that are in use in our simulation, as well as the capacitance model used for the simulation purposes, as well as some coupling between the quantum dot gates and the charge sensor.
These values are best described in the QArray simulation.

Then we initialize the database schema.
The purpose of initializing the schema is to restart the global variables that this autotuner runs on.
We must resupply them for each test since these global variables parameterize the expected behavior.
These variables in the database are the ones that developers should expect that end users will be setting via an external yaml configuration for the entire falcon autotuner.
As such, make sure that your external environmental variables are compliant with other externally used modules.
In this case the global variables that this simple demo depends on are:

- virtualization: the virtual gates matrix for the DUT
- resolution: the number of points to take in a single 1D sweep
- virtualizationName: the names of the indexes of the virtual gates matrix
- interdotSpacing: the expected spacing of peaks in the virtualized gate voltage space for each dimension

Next, we initialize the starting conditions for the simulation.
We assume that we are tuned into the (0,0) charge configuration at the beginning of this autotuning session.
In this case we can tune to P1=0.0V and P2=0.0V.
We also detune our sensing dot to improve our SNR to S1=-0.1V.

Finally, we run the test autotuner by defining the variable types and running the autotuner.
Then we print the info about the ending location using the log package.

### The Charge Configuration Tuner

This autotuner focuses on navigating the high level charge stability diagram in two dimensions.
The autotuner itself contains 2 state variables, 'n' and 'm', that describe the current tuned state.
It then initialized tuning in the upwards direction on the stability diagram, and each time through it re-runs the tuning state with a new initial condition from the run state.
Each state launches a BlipStateStepper, which is responsible for tuning the DUT into the next selected charge state.

### The Blip State Stepper

This autotuner manages moving the DUT in the direction of the next selected charge state.
It accomplishes this by first unpacking all the global variables that we injected from the test in the checkDatabaseEntries state.
Then it unpacks all the InterDot spacings loaded since it needs to for loop through them in the state unpackInterdotSpacing.
Finally it performs the step by first analyzing the blip state, and then calculating a difference between its current state and the midpoint of teh found blips and ramps the device there.

### The Hub

You might notice that there were hub module routines that were called in the Blip State Stepper autotuner.
These routines are strongly typed in the hub file in the demo.
However, what might be surprising is that there are no implementations of any code in that file.
This is since the hub file is using our FFI interface to call functions from outside of the FAlCon DSL to accomplish tasks outside the DSL.
This is expected since implementing direct memory access is not the task of the DSL but is regulated to lower level implementation details.
Where authors decide to place this division is up to them.
It is however advantageous to keep as much code in the DSL as possible since it is much more interpretable and reuse-able.
This FFI interface uses the FFI-helpers, and the implementation is in the qarray-wrapper.
To understand more about this interface return to our standard [documentation](https://github.com/falcon-autotuning/falcon-lib/tree/main/typing).
We won't be going into much detail about the implementation here, but the details are there for your viewing pleasure.

## Support

For questions or issues, please open an issue in the repository.
