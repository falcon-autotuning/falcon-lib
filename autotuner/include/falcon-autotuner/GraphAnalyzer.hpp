#pragma once

#include "falcon-autotuner/Autotuner.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>
#include <set>
#include <sstream>

namespace falcon::autotuner {

/**
 * @brief Analysis result for autotuner graph
 */
struct GraphAnalysisResult {
  bool has_cycles = false;
  std::vector<std::string> cycle_path;
  size_t total_nodes = 0;
  size_t max_depth = 0;
  std::string error_message;

  [[nodiscard]] bool is_valid() const {
    return !has_cycles && error_message.empty();
  }
};

/**
 * @brief Analyzes autotuner graphs for structural issues
 */
class GraphAnalyzer {
public:
  /**
   * @brief Analyze an autotuner for cycles and other issues
   */
  static GraphAnalysisResult analyze(const Autotuner &autotuner) {
    GraphAnalysisResult result;

    if (!autotuner.root()) {
      result.error_message = "Autotuner has no root node";
      return result;
    }

    // Build graph representation
    std::map<AutotunerNode *, size_t> node_indices;
    std::vector<AutotunerNode *> nodes;

    build_node_graph(autotuner.root().get(), nodes, node_indices);
    result.total_nodes = nodes.size();

    // Check for cycles using DFS
    std::set<AutotunerNode *> visited;
    std::set<AutotunerNode *> rec_stack;
    std::vector<AutotunerNode *> path;

    if (has_cycle_dfs(autotuner.root().get(), visited, rec_stack, path)) {
      result.has_cycles = true;
      for (auto *node : path) {
        result.cycle_path.push_back(node->name());
      }
    }

    // Calculate max depth
    result.max_depth = calculate_max_depth(autotuner.root().get(), 0);

    return result;
  }

  /**
   * @brief Generate a DOT graph for visualization
   */
  static std::string to_dot(const Autotuner &autotuner) {
    std::ostringstream oss;
    oss << "digraph Autotuner {\n";
    oss << "  label=\"" << autotuner.name() << "\";\n";
    oss << "  rankdir=TB;\n\n";

    std::set<AutotunerNode *> visited;
    size_t node_counter = 0;
    std::map<AutotunerNode *, size_t> node_ids;

    if (autotuner.root()) {
      generate_dot_nodes(autotuner.root().get(), oss, visited, node_counter,
                         node_ids);
    }

    oss << "}\n";
    return oss.str();
  }

private:
  static void
  build_node_graph(AutotunerNode *node, std::vector<AutotunerNode *> &nodes,
                   std::map<AutotunerNode *, size_t> &node_indices) {

    if (node_indices.find(node) != node_indices.end()) {
      return; // Already visited
    }

    node_indices[node] = nodes.size();
    nodes.push_back(node);

    for (auto &child : node->children()) {
      build_node_graph(child.get(), nodes, node_indices);
    }
  }

  static bool has_cycle_dfs(AutotunerNode *node,
                            std::set<AutotunerNode *> &visited,
                            std::set<AutotunerNode *> &rec_stack,
                            std::vector<AutotunerNode *> &path) {

    if (rec_stack.find(node) != rec_stack.end()) {
      // Found cycle
      path.push_back(node);
      return true;
    }

    if (visited.find(node) != visited.end()) {
      return false; // Already fully explored
    }

    visited.insert(node);
    rec_stack.insert(node);
    path.push_back(node);

    for (auto &child : node->children()) {
      if (has_cycle_dfs(child.get(), visited, rec_stack, path)) {
        return true;
      }
    }

    rec_stack.erase(node);
    path.pop_back();
    return false;
  }

  static size_t calculate_max_depth(AutotunerNode *node, size_t current_depth) {
    auto children = node->children();
    if (children.empty()) {
      return current_depth;
    }

    size_t max_child_depth = current_depth;
    for (auto &child : children) {
      size_t child_depth = calculate_max_depth(child.get(), current_depth + 1);
      max_child_depth = std::max(max_child_depth, child_depth);
    }

    return max_child_depth;
  }

  static void generate_dot_nodes(AutotunerNode *node, std::ostringstream &oss,
                                 std::set<AutotunerNode *> &visited,
                                 size_t &node_counter,
                                 std::map<AutotunerNode *, size_t> &node_ids) {

    if (visited.find(node) != visited.end()) {
      return;
    }

    visited.insert(node);
    size_t node_id = node_counter++;
    node_ids[node] = node_id;

    // Node styling based on type
    std::string shape = "box";
    std::string color = "black";

    switch (node->type()) {
    case NodeType::Measurement:
      shape = "ellipse";
      color = "blue";
      break;
    case NodeType::Nested:
      shape = "box";
      color = "red";
      break;
    case NodeType::Iterator:
      shape = "diamond";
      color = "green";
      break;
    }

    oss << "  node" << node_id << " [label=\"" << node->name()
        << "\", shape=" << shape << ", color=" << color << "];\n";

    // Generate edges to children
    for (auto &child : node->children()) {
      // Ensure child is generated first
      if (visited.find(child.get()) == visited.end()) {
        generate_dot_nodes(child.get(), oss, visited, node_counter, node_ids);
      }

      size_t child_id = node_ids[child.get()];
      oss << "  node" << node_id << " -> node" << child_id << ";\n";
    }
  }
};

} // namespace falcon::autotuner
