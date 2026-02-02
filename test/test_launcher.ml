

open Falcon
open Acu_templates

let () =
  Acu_registry.register "LoggerToy" (module ClassBridge(struct let create h r i m c d = (new Toy_acus.logger_toy h r i m c d :> Base_acu.base_algorithmic_control_unit) end));
  Acu_registry.register "SpawnerToy" (module ClassBridge(struct let create h r i m c d = (new Toy_acus.spawner_toy h r i m c d :> Base_acu.base_algorithmic_control_unit) end));
  Acu_registry.register "GraphAutotunerToy" (module ClassBridge(struct let create h r i m c d = (new Toy_acus.graph_autotuner_toy h r i m c d :> Base_acu.base_algorithmic_control_unit) end));
  Acu_registry.register "LinearOptimizerToy" (module ClassBridge(struct let create h r i m c d = (new Toy_acus.linear_optimizer_toy h r i m c d :> Base_acu.base_algorithmic_control_unit) end));
  Acu_registry.register "BridgeAnalyzerToy" (module ClassBridge(struct let create h r i m c d = (new Toy_acus.bridge_analyzer_toy h r i m c d :> Base_acu.base_algorithmic_control_unit) end));
  Acu_registry.register "FormalDependencyToy" (module ClassBridge(struct let create h r i m c d = (new Toy_acus.formal_dependency_toy h r i m c d :> Base_acu.base_algorithmic_control_unit) end));
  
  Lwt_main.run (Falcon.Launcher_logic.run ())
