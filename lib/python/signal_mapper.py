import numpy as np

class SignalMapper:
    """
    Utility for resolving semantic signal keys from raw measurement data
    using a provided mapping configuration.
    """
    def __init__(self, signals, mapping=None):
        self.signals = signals
        self.mapping = mapping or {}

    def get_signal(self, semantic_name, default=None):
        """
        Retrieves a signal by its semantic name (e.g., 'current').
        Checks mapping first, then falls back to direct key lookup.
        """
        raw_key = self.mapping.get(semantic_name, semantic_name)
        val = self.signals.get(raw_key)
        
        if val is None:
            return default
            
        return np.array(val)

    def get_contextual_value(self, context, key, default=None):
        """
        Helper to extract values from dependencies or instance_vars.
        """
        # check dependencies first (DeviceCharacteristics)
        dep = context.get('dependencies', {}).get(key)
        if dep is not None:
            return dep.get('value', default)
            
        # fall back to instance_vars
        return context.get('instance_vars', {}).get(key, default)
