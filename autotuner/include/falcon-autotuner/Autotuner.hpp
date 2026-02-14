#pragma once

#include "falcon-autotuner/MeasurementRoutine.hpp"
#include "falcon-autotuner/State.hpp"
#include "falcon-autotuner/StateIterators.hpp"
#include <boost/sml.hpp>
#include <memory>
#include <vector>

namespace falcon::autotuner {

namespace sml = boost::sml;

/**
 * @brief Events for state machine transitions
 */
struct NextEvent {};
struct SuccessEvent {};
struct FailureEvent {};
struct ExitEvent {
  bool success;
};

/**
 * @brief Forward declarations
 */
class Autotuner;
class AutotunerNode;

/**
 * @brief Types of nodes in the autotuner graph
 */
enum class NodeType : uint8_t {
  Measurement, // Leaf node - executes measurement
  Nested,      // Internal node - executes nested autotuner
  Iterator     // Wrapper node - iterates over states
};

/**
 * @brief Represents a node in the autotuner execution graph
 */
class AutotunerNode {
public:
  virtual ~AutotunerNode() = default;
  [[nodiscard]] virtual NodeType type() const = 0;
  [[nodiscard]] virtual std::string name() const = 0;

  /**
   * @brief Execute this node with given state
   * @return Result indicating next action
   */
  virtual MeasurementResult execute(State &state) = 0;

  /**
   * @brief Get child nodes (for graph analysis)
   */
  [[nodiscard]] virtual std::vector<std::shared_ptr<AutotunerNode>>
  children() const {
    return {};
  }
};

/**
 * @brief Measurement node - executes a measurement routine
 */
class MeasurementNode : public AutotunerNode {
public:
  explicit MeasurementNode(std::shared_ptr<MeasurementRoutine> routine)
      : routine_(std::move(routine)) {}

  [[nodiscard]] NodeType type() const override { return NodeType::Measurement; }
  [[nodiscard]] std::string name() const override { return routine_->name(); }

  MeasurementResult execute(State &state) override {
    return routine_->execute(state.get_combined_params());
  }

private:
  std::shared_ptr<MeasurementRoutine> routine_;
};

/**
 * @brief Nested autotuner node - executes another autotuner
 */
class NestedAutotunerNode : public AutotunerNode {
public:
  explicit NestedAutotunerNode(std::shared_ptr<Autotuner> autotuner);

  [[nodiscard]] NodeType type() const override { return NodeType::Nested; }
  [[nodiscard]] std::string name() const override;

  MeasurementResult execute(State &state) override;

  [[nodiscard]] std::vector<std::shared_ptr<AutotunerNode>>
  children() const override;

private:
  std::shared_ptr<Autotuner> autotuner_;
};

/**
 * @brief Iterator node - wraps a node and executes it for each value
 */
template <typename T> class IteratorNode : public AutotunerNode {
public:
  IteratorNode(std::string param_name,
               std::shared_ptr<StateIterator<T>> iterator,
               std::shared_ptr<AutotunerNode> child)
      : param_name_(std::move(param_name)), iterator_(std::move(iterator)),
        child_(std::move(child)) {}

  [[nodiscard]] NodeType type() const override { return NodeType::Iterator; }
  [[nodiscard]] std::string name() const override {
    return "Iterator(" + param_name_ + ") -> " + child_->name();
  }

  MeasurementResult execute(State &state) override {
    iterator_->reset();

    while (iterator_->has_next()) {
      T value = iterator_->next();
      state.local_params().set(param_name_, value);

      auto result = child_->execute(state);

      // Handle exit conditions
      if (result.next_action != MeasurementResult::Action::Continue) {
        return result;
      }
    }

    return MeasurementResult{};
  }

  [[nodiscard]] std::vector<std::shared_ptr<AutotunerNode>>
  children() const override {
    return {child_};
  }

private:
  std::string param_name_;
  std::shared_ptr<StateIterator<T>> iterator_;
  std::shared_ptr<AutotunerNode> child_;
};

/**
 * @brief Main Autotuner class - manages state machine execution
 */
class Autotuner : public std::enable_shared_from_this<Autotuner> {
public:
  explicit Autotuner(std::string name) : name_(std::move(name)) {}

  const std::string &name() const { return name_; }

  /**
   * @brief Set the root execution node
   */
  void set_root(std::shared_ptr<AutotunerNode> root) {
    root_ = std::move(root);
  }

  /**
   * @brief Get the root node
   */
  std::shared_ptr<AutotunerNode> root() const { return root_; }

  /**
   * @brief Execute the autotuner with given initial parameters
   */
  MeasurementResult run(const ParameterMap &initial_params = ParameterMap()) {
    State state(name_);
    state.set_parent_params(initial_params);

    if (!root_) {
      throw std::runtime_error("Autotuner has no root node");
    }

    return root_->execute(state);
  }

  /**
   * @brief Create a measurement node
   */
  static std::shared_ptr<AutotunerNode>
  measurement(std::shared_ptr<MeasurementRoutine> routine) {
    return std::make_shared<MeasurementNode>(std::move(routine));
  }

  /**
   * @brief Create a nested autotuner node
   */
  static std::shared_ptr<AutotunerNode>
  nested(std::shared_ptr<Autotuner> autotuner) {
    return std::make_shared<NestedAutotunerNode>(std::move(autotuner));
  }

  /**
   * @brief Create an iterator node
   */
  template <typename T>
  static std::shared_ptr<AutotunerNode>
  iterate(std::string param_name, std::shared_ptr<StateIterator<T>> iterator,
          std::shared_ptr<AutotunerNode> child) {
    return std::make_shared<IteratorNode<T>>(
        std::move(param_name), std::move(iterator), std::move(child));
  }

  /**
   * @brief Helper to create list iterator
   */
  template <typename T>
  static std::shared_ptr<AutotunerNode>
  iterate_list(std::string param_name, std::vector<T> values,
               std::shared_ptr<AutotunerNode> child) {
    auto iterator = std::make_shared<ListIterator<T>>(std::move(values));
    return iterate(std::move(param_name), iterator, std::move(child));
  }

  /**
   * @brief Helper to create integer range iterator
   */
  static std::shared_ptr<AutotunerNode>
  iterate_range(const std::string &param_name, int64_t start, int64_t end,
                int64_t step, const std::shared_ptr<AutotunerNode> &child) {
    auto iterator = std::make_shared<IntegerRangeIterator>(start, end, step);
    return iterate<int64_t>(param_name, iterator, child);
  }

  /**
   * @brief Helper to create float range iterator
   */
  static std::shared_ptr<AutotunerNode>
  iterate_float_range(const std::string &param_name, double start, double end,
                      size_t divisions,
                      const std::shared_ptr<AutotunerNode> &child) {
    auto iterator = std::make_shared<FloatRangeIterator>(start, end, divisions);
    return iterate<double>(param_name, iterator, child);
  }

private:
  std::string name_;
  std::shared_ptr<AutotunerNode> root_;
};

// Implementations of NestedAutotunerNode methods
inline NestedAutotunerNode::NestedAutotunerNode(
    std::shared_ptr<Autotuner> autotuner)
    : autotuner_(std::move(autotuner)) {}

inline std::string NestedAutotunerNode::name() const {
  return "Nested(" + autotuner_->name() + ")";
}

inline MeasurementResult NestedAutotunerNode::execute(State &state) {
  // Pass parent state as read-only parameters to nested autotuner
  return autotuner_->run(state.get_combined_params());
}

inline std::vector<std::shared_ptr<AutotunerNode>>
NestedAutotunerNode::children() const {
  if (autotuner_->root()) {
    return {autotuner_->root()};
  }
  return {};
}

} // namespace falcon::autotuner
