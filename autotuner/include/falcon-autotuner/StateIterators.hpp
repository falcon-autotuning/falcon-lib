#pragma once

#include <functional>
#include <optional>
#include <stdexcept>
#include <vector>

namespace falcon::autotuner {

/**
 * @brief Base class for state parameter iterators
 */
template <typename T> class StateIterator {
public:
  virtual ~StateIterator() = default;
  [[nodiscard]] virtual bool has_next() const = 0;
  virtual T next() = 0;
  virtual void reset() = 0;
  [[nodiscard]] virtual size_t size() const = 0;
};

/**
 * @brief Iterator over a list of values
 */
template <typename T> class ListIterator : public StateIterator<T> {
public:
  explicit ListIterator(std::vector<T> values) : values_(std::move(values)) {}

  [[nodiscard]] bool has_next() const override {
    return current_ < values_.size();
  }

  T next() override {
    if (!has_next()) {
      throw std::out_of_range("ListIterator: no more elements");
    }
    return values_[current_++];
  }

  void reset() override { current_ = 0; }

  [[nodiscard]] size_t size() const override { return values_.size(); }

private:
  std::vector<T> values_;
  size_t current_{};
};

/**
 * @brief Iterator over a range of integers
 */
class IntegerRangeIterator : public StateIterator<int64_t> {
public:
  IntegerRangeIterator(int64_t start, int64_t end, int64_t step = 1)
      : start_(start), end_(end), step_(step), current_(start) {
    if (step == 0) {
      throw std::invalid_argument("Step cannot be zero");
    }
  }

  [[nodiscard]] bool has_next() const override {
    if (step_ > 0) {
      return current_ < end_;
    } else {
      return current_ > end_;
    }
  }

  int64_t next() override {
    if (!has_next()) {
      throw std::out_of_range("IntegerRangeIterator: no more elements");
    }
    auto value = current_;
    current_ += step_;
    return value;
  }

  void reset() override { current_ = start_; }

  [[nodiscard]] size_t size() const override {
    if (step_ > 0) {
      return (end_ - start_ + step_ - 1) / step_;
    } else {
      return (start_ - end_ - step_ - 1) / (-step_);
    }
  }

private:
  int64_t start_;
  int64_t end_;
  int64_t step_;
  int64_t current_;
};

/**
 * @brief Iterator over evenly divided float range
 */
class FloatRangeIterator : public StateIterator<double> {
public:
  FloatRangeIterator(double start, double end, size_t divisions)
      : start_(start), end_(end), divisions_(divisions) {
    if (divisions == 0) {
      throw std::invalid_argument("Divisions cannot be zero");
    }
    step_ = (end - start) / static_cast<double>(divisions);
  }

  [[nodiscard]] bool has_next() const override {
    return current_ <= divisions_;
  }

  double next() override {
    if (!has_next()) {
      throw std::out_of_range("FloatRangeIterator: no more elements");
    }
    double value = start_ + step_ * static_cast<double>(current_);
    current_++;
    return value;
  }

  void reset() override { current_ = 0; }

  [[nodiscard]] size_t size() const override { return divisions_ + 1; }

private:
  double start_;
  double end_;
  size_t divisions_;
  double step_;
  size_t current_{};
};

/**
 * @brief Iterator with custom generator function
 */
template <typename T> class GeneratorIterator : public StateIterator<T> {
public:
  GeneratorIterator(std::function<std::optional<T>()> generator,
                    size_t estimated_size = 0)
      : generator_(std::move(generator)), size_(estimated_size) {}

  bool has_next() const override {
    if (!next_value_.has_value()) {
      next_value_ = generator_();
    }
    return next_value_.has_value();
  }

  T next() override {
    if (!has_next()) {
      throw std::out_of_range("GeneratorIterator: no more elements");
    }
    T value = std::move(*next_value_);
    next_value_.reset();
    return value;
  }

  void reset() override {
    // Cannot reset generator-based iterator
    throw std::runtime_error("GeneratorIterator cannot be reset");
  }

  size_t size() const override { return size_; }

private:
  std::function<std::optional<T>()> generator_;
  mutable std::optional<T> next_value_;
  size_t size_;
};

} // namespace falcon::autotuner
