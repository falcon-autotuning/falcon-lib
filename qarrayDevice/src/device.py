from pathlib import Path
from typing import Any

import numpy as np
import yaml
from qarray import ChargeSensedDotArray, DotArray

DEFAULT_VOLTAGE = 0.0


class Device:
    """
    A class to represent a quantum dot device, configured via a YAML file.

    This class acts as a wrapper around qarray's DotArray or ChargeSensedDotArray,
    allowing for easy configuration and control.
    """

    n_dots: int
    gate_names: list[str]
    voltages: dict[str, float]

    def __init__(self, config: str | dict[str, "Any"]):
        """
        Initializes the Device from a YAML configuration file.

        Args:
            config_path: The path to the YAML configuration file.
        """
        if isinstance(config, str):
            config_loc = Path(config)
            with config_loc.open("r") as f:
                config = yaml.safe_load(f)
        assert not isinstance(config, str), "The config did not get converted propertly"
        self.n_dots = int(config["n_dots"])
        self.gate_names = config["gate_names"]
        self.voltages = {name: DEFAULT_VOLTAGE for name in self.gate_names}
        self.sensor_config = config.get("sensor_config")
        self.capacitances_config = config.get("capacitances", {})

        Cdd, Cgd, Cds, Cgs = self._load_or_generate_capacitances(config)

        if self.sensor_config:
            self.model = ChargeSensedDotArray(
                Cdd=Cdd,
                Cgd=Cgd,
                Cds=Cds,
                Cgs=Cgs,
                charge_carrier="electrons",
                T=0.1,
            )
            # self.model.charge_carrier = "electrons"
            # self.model.update_capacitance_matrices(Cdd, Cgd, Cds, Cgs)
        else:
            self.model = DotArray(
                Cdd=Cdd,
                Cgd=Cgd,
                charge_carrier="electron",
                T=0.1,
            )

    def _load_or_generate_capacitances(self, config_dir: Path | dict[str, Any]):
        """Loads capacitance matrices from .npy files, YAML, or generates default ones."""

        def load_or_get_matrix(key):
            """Load from .npy if value is a string, else assume it's a list/array."""
            val = self.capacitances_config.get(key)
            if val is None:
                return None
            if isinstance(val, str) and isinstance(config_dir, Path):
                return np.load(config_dir / val)
            return np.array(val)

        Cdd = load_or_get_matrix("Cdd")
        Cgd = load_or_get_matrix("Cgd")
        Cds = load_or_get_matrix("Cds")
        Cgs = load_or_get_matrix("Cgs")

        if Cdd is None:
            Cdd = np.full((self.n_dots, self.n_dots), 0.1)
            np.fill_diagonal(Cdd, 0.0)

        if Cgd is None:
            n_gates = len(self.gate_names)
            Cgd = np.full((self.n_dots, n_gates), 0.2)
            # Assume plunger gates are the first n_dots gates
            plunger_indices = range(min(self.n_dots, n_gates))
            Cgd[plunger_indices, plunger_indices] = 1.0

        if self.sensor_config:
            n_sensors = len(self.sensor_config)
            if Cds is None:
                Cds = np.full((n_sensors, self.n_dots), 0.01)
            if Cgs is None:
                n_gates = len(self.gate_names)
                Cgs = np.full((n_sensors, n_gates), 0.05)
                # Set high coupling for sensor gates to their respective sensors
                for i, sensor_gate in enumerate(self.sensor_config.keys()):
                    if sensor_gate in self.gate_names:
                        gate_idx = self.gate_names.index(sensor_gate)
                        Cgs[i, gate_idx] = 1.0

        return Cdd, Cgd, Cds, Cgs

    def set_voltage(self, gate_name: str, value: float):
        """Sets the voltage for a single specified gate."""
        if gate_name not in self.gate_names:
            raise ValueError(f"Gate '{gate_name}' not found in device configuration.")
        self.voltages[gate_name] = value

    def set_voltages(self, voltage_dict: dict[str, float]):
        """Sets voltages for multiple gates from a dictionary."""
        for gate_name, value in voltage_dict.items():
            self.set_voltage(gate_name, value)

    def scan_2d(
        self,
        x_gate: str,
        y_gate: str,
        x_range: tuple[float, float],
        y_range: tuple[float, float],
        resolution: int,
    ):
        """
        Performs a 2D voltage scan and simulates the device response.



        Args:
            x_gate: Name of the gate for the x-axis sweep.
            y_gate: Name of the gate for the y-axis sweep.
            x_range: A tuple (min, max) for the x-axis voltage sweep.
            y_range: A tuple (min, max) for the y-axis voltage sweep.
            resolution: The number of points for both x and y dimensions.

        Returns:
            A dictionary containing the simulation results and metadata for plotting.
        """
        # FIXME: This does not follow time inversion symetry. The sign of the gradient will change depending on sweep direction. It should not in a real device.
        x_min, x_max = x_range
        y_min, y_max = y_range

        vg = self.model.gate_voltage_composer.do2d(
            x_gate, x_min, x_max, resolution, y_gate, y_min, y_max, resolution
        )
        base_voltages = np.array([self.voltages[g] for g in self.gate_names])[
            np.newaxis, np.newaxis, :
        ]
        # Apply DC offsets
        try:
            vg += base_voltages
        except Exception as e:
            raise ValueError(
                f"The model's n_gates is: {self.model.n_gate}. Sim output shape was {vg.shape} but DAC base voltages shape was {base_voltages.shape}. The error was {e}"
            )

        result = {
            "x_gate": x_gate,
            "y_gate": y_gate,
            "x_range": x_range,
            "y_range": y_range,
            "resolution": resolution,
        }

        if self.sensor_config:
            sensor_output, charge_states = self.model.charge_sensor_open(vg)
            result["sensor_output"] = sensor_output
            result["charge_states"] = charge_states
            dz_dv = np.gradient(sensor_output, axis=0) + np.gradient(
                sensor_output, axis=1
            )
            result["differentiated_signal"] = dz_dv
        else:
            charge_states = self.model.ground_state_open(vg)
            result["charge_states"] = charge_states

        return result

    def scan_1d(self, gate: str, v_range: tuple, resolution: int):
        """
        Performs a 1D voltage sweep on a single gate.

        Args:
            gate: The name of the gate to sweep.
            v_range: A tuple (min_voltage, max_voltage) defining the voltage range for the sweep.
            resolution: The number of points in the sweep.

        Returns:
            A dictionary containing 1D sweep results, including gate voltages and simulation results.
        """
        if gate not in self.gate_names:
            raise ValueError(f"Gate '{gate}' not found in device configuration.")

        v_min, v_max = v_range

        vg = self.model.gate_voltage_composer.do1d(
            gate=gate, min=v_min, max=v_max, res=resolution
        )

        # Apply DC offsets
        base_voltages = np.array([self.voltages[g] for g in self.gate_names])
        vg += base_voltages

        # Apply the scan to the target gate

        result = {"gate": gate, "v_range": v_range, "resolution": resolution}

        dv = np.gradient(
            np.linspace(
                v_min,
                v_max,
                resolution,
            )
        ).reshape(-1, 1)
        if self.sensor_config:
            sensor_output, charge_states = self.model.charge_sensor_open(vg)
            result["sensor_output"] = sensor_output
            result["charge_states"] = charge_states
            dz_dv = np.gradient(sensor_output, axis=0) / dv
            result["differentiated_signal"] = dz_dv
        else:
            charge_states = self.model.ground_state_open(vg)
            result["charge_states"] = charge_states

        return result

    def scan_ray(self, start: dict[str, float], end: dict[str, float], resolution: int):
        """
        Performs a ray-based voltage scan between two points in 2D gate voltage space.

        Args:
            start: A dictionary with gate names as keys and starting voltages as values.
            end: A dictionary with gate names as keys and ending voltages as values.
            resolution: The number of points along the ray.

        Returns:
            A dictionary containing the ray sweep results, with gate voltages and simulation results.
        """
        # Validate that all gates in start and end are valid
        for gate in start.keys():
            if gate not in self.gate_names:
                raise ValueError(f"Gate '{gate}' not found in device configuration.")
        for gate in end.keys():
            if gate not in self.gate_names:
                raise ValueError(f"Gate '{gate}' not found in device configuration.")

        start_w_context = {
            conn: start.get(conn, self.voltages[conn]) for conn in self.gate_names
        }
        end_w_context = {
            conn: end.get(conn, self.voltages[conn]) for conn in self.gate_names
        }

        # Compute the voltage trajectory
        start_volts = np.array([start_w_context[conn] for conn in self.gate_names])
        end_volts = np.array([end_w_context[conn] for conn in self.gate_names])
        vg_all = np.linspace(start_volts, end_volts, resolution)

        vector = {
            connection: float(end_w_context[connection] - start_w_context[connection])
            for connection in self.gate_names
        }
        vector = {
            connection: component
            for connection, component in vector.items()
            if not bool(np.isclose(component, 0))
        }
        magnitude = float(
            np.sqrt(np.sum([component**2 for component in vector.values()]))
        )
        direction = {
            connection: vector[connection] / magnitude for connection in vector
        }
        dvs = {
            connection: np.gradient(
                np.linspace(
                    # TODO: A proper gradient would respect the start and stop in correct ordering
                    min(start_w_context[connection], end_w_context[connection]),
                    max(start_w_context[connection], end_w_context[connection]),
                    resolution,
                )
            ).reshape(-1, 1)
            for connection in vector
        }
        # print(f"The dvs are {dvs}")

        result = {"start": start, "end": end, "resolution": resolution}
        # print(f"this is the vg_all {vg_all}")
        # print(f"this is the result dict {result}")
        if self.sensor_config:
            sensor_output, charge_states = self.model.charge_sensor_open(vg_all)
            result["sensor_output"] = sensor_output
            result["charge_states"] = charge_states
            # TODO: The sign of the gradient should depend on the slope of the charge sensor when this measurement was taken
            dz = np.gradient(sensor_output, axis=0)
            dz_dv = np.sum(
                [direction[conn] * dz / dvs[conn] for conn in vector],
                axis=0,
            )
            # print(f"The shape of dz is {dz.shape}")
            # print(f"The shape of dvs is {[dv.shape for dv in dvs.values()]}")
            # print(f"The shape of dz_dv is {dz_dv.shape}")
            result["differentiated_signal"] = dz_dv
            # print("did a charge sensed thing in rays")
        else:
            charge_states = self.model.ground_state_open(vg_all)
            result["charge_states"] = charge_states
            # print("did a not charge sensed thing in rays")
        result["trajectory"] = vg_all  # Include the trajectory for inspection
        return result
