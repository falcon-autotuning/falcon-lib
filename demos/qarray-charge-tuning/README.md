# QArray Charge Tuning Demo

## Overview

This demo showcases QArray charge tuning in FAlCon, leveraging a device characteristics database for global variable management.  
**Supported Platforms:** Linux (WSL may work)

## Key Components

- **ChargeConfigurationTuner**: High-level charge system autotuning.
- **BlipStateStepper**: Fine-grained control for crossing charge boundaries.

## Running the Demo

### Prerequisites

- Linux or WSL
- PostgreSQL
- FAlCon libraries installed at `/opt/falcon/lib`

### Environment Setup

Set the required environment variables:

```bash
export PATH="/opt/falcon/lib:$PATH"
export LD_LIBRARY_PATH="/opt/falcon/lib:$LD_LIBRARY_PATH"
export PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig:$PKG_CONFIG_PATH"
```

### Database Setup

Ensure PostgreSQL is running and create the Falcon test database:

```bash
export TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@127.0.0.1:5432/falcon_test"
```

### Demo Execution

Clone the repository and navigate to the demo folder:

```bash
cd demos/qarray-charge-tuning
make docker-up
falcon-test ./tests/run_tests.fal --log-level info
```

- Adjust `--log-level` for desired verbosity (e.g., `info`, `debug`).

### Variable Scoping is important

When viewing the example code for charge stability diagrams, you may notice that variables are defined at different scopes.
Here is an example of how variables can be defined at the autotuner scope and state scope:

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

- In the `init` state, both `autotuner_scope_variable` and `state_scope_variable` are accessible.
- The autotuner itself does not know about `state_scope_variable`.

For complex scenarios (e.g., sweep resolution), variables can be injected at higher scopes using the PostgreSQL database.  
While a CLI exists for real deployments, this demo constructs them directly in the test.

---

For questions or issues, please open an issue in the repository.

```
This version organizes content into sections, improves formatting, and follows traditional demo README structure. Add images using Markdown syntax if needed:
```markdown
![Charge Stability Diagram](images/charge_stability.png)
```
