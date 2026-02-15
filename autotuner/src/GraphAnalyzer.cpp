#include "falcon-autotuner/GraphAnalyzer.hpp"
#include <queue>

namespace falcon {
namespace autotuner {

GraphAnalysisResult GraphAnalyzer::analyze(const Autotuner& autotuner) {
    GraphAnalysisResult result;
    
    auto states = autotuner.get_all_states();
    result.total_states = states.size();
    
    // Count transitions and find terminals
    for (const auto& [name, state] : states) {
        result.total_transitions += state->transitions().size();
        if (state->is_terminal()) {
            result.terminal_states.push_back(state->id());
        }
    }
    
    // Check for cycles
    std::set<StateId> visited;
    std::set<StateId> rec_stack;
    std::vector<std::string> path;
    
    if (detect_cycles(autotuner, autotuner.entry_state(), visited, rec_stack, path)) {
        result.has_cycles = true;
        result.cycle_path = path;
    }
    
    // Find reachable states
    auto reachable = find_reachable_states(autotuner);
    
    for (const auto& [name, state] : states) {
        if (reachable.find(state->id()) == reachable.end()) {
            result.unreachable_states.push_back(state->id());
        }
    }
    
    return result;
}

bool GraphAnalyzer::detect_cycles(
    const Autotuner& autotuner,
    const StateId& current,
    std::set<StateId>& visited,
    std::set<StateId>& rec_stack,
    std::vector<std::string>& path) {
    
    // Check if we're in a cycle
    if (rec_stack.find(current) != rec_stack.end()) {
        path.push_back(current.full_name());
        return true;
    }
    
    // Already visited and checked
    if (visited.find(current) != visited.end()) {
        return false;
    }
    
    visited.insert(current);
    rec_stack.insert(current);
    path.push_back(current.full_name());
    
    // Check all transitions
    auto state = autotuner.get_state(current.state_name);
    if (state) {
        for (const auto& transition : state->transitions()) {
            if (detect_cycles(autotuner, transition.target, visited, rec_stack, path)) {
                return true;
            }
        }
    }
    
    rec_stack.erase(current);
    path.pop_back();
    return false;
}

std::set<StateId> GraphAnalyzer::find_reachable_states(const Autotuner& autotuner) {
    std::set<StateId> reachable;
    std::queue<StateId> to_visit;
    
    to_visit.push(autotuner.entry_state());
    reachable.insert(autotuner.entry_state());
    
    while (!to_visit.empty()) {
        StateId current = to_visit.front();
        to_visit.pop();
        
        auto state = autotuner.get_state(current.state_name);
        if (state) {
            for (const auto& transition : state->transitions()) {
                if (reachable.find(transition.target) == reachable.end()) {
                    reachable.insert(transition.target);
                    to_visit.push(transition.target);
                }
            }
        }
    }
    
    return reachable;
}

} // namespace autotuner
} // namespace falcon
