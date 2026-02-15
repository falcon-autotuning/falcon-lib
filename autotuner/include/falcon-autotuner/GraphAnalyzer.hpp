#pragma once

#include "Autotuner.hpp"
#include <set>
#include <vector>

namespace falcon {
namespace autotuner {

/**
 * @brief Analysis result for a state machine
 */
struct GraphAnalysisResult {
  bool has_cycles = false;
  std::vector<std::string> cycle_path;
  std::vector<StateId> unreachable_states;
  std::vector<StateId> terminal_states;
  int total_states = 0;
  int total_transitions = 0;

  bool is_valid() const { return !has_cycles && unreachable_states.empty(); }
};

/**
 * @brief Analyzer for autotuner state machines
 */
class GraphAnalyzer {
public:
  /**
   * @brief Analyze an autotuner's state machine
   */
  static GraphAnalysisResult analyze(const Autotuner &autotuner);

private:
  static bool detect_cycles(const Autotuner &autotuner, const StateId &current,
                            std::set<StateId> &visited,
                            std::set<StateId> &rec_stack,
                            std::vector<std::string> &path);

  static std::set<StateId> find_reachable_states(const Autotuner &autotuner);
};

} // namespace autotuner
} // namespace falcon
