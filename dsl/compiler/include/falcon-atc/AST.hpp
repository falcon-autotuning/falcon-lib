#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace falcon::atc {

// ============================================================================
// TYPE SYSTEM
// ============================================================================

/**
 * @brief Core type enumeration for the Falcon autotuner language.
 *
 * All types are defined by the falcon-core library. Users cannot define
 * new types - they can only use these predefined types in their autotuners.
 *
 * This constraint simplifies the language while providing all necessary
 * functionality for autotuning workflows.
 */
enum class ParamType : std::uint8_t {
  Int,    // 64-bit signed integer
  Float,  // 64-bit floating point (double precision)
  Bool,   // Boolean (true/false)
  String, // UTF-8 string
  // Error handling
  Error, // Error value (can be nil or contain error info)

  Nil,   // Represents null/absence (only for Error type checking)
  Tuple, // Multiple return values (e.g., (int, Error))
  Void,  // No return value (for procedures with side effects only)
  Union,
  Struct, // User-defined struct type; name stored in
          // TypeDescriptor::struct_name
  Array,  // Generic array type; element type stored in
          // TypeDescriptor::element_type
};

inline std::string to_string(ParamType type) {
  switch (type) {
  case ParamType::Int:
    return "int";
  case ParamType::Float:
    return "float";
  case ParamType::Bool:
    return "bool";
  case ParamType::String:
    return "string";
  case ParamType::Error:
    return "Error";
  case ParamType::Nil:
    return "nil";
  case ParamType::Tuple:
    return "tuple";
  case ParamType::Void:
    return "void";
  case ParamType::Struct:
    return "struct";
  case ParamType::Array:
    return "Array";
  default:
    return "<unknown>";
  }
}

/**
 * @brief Type descriptor that can represent simple or compound types.
 *
 * Most types are simple (ParamType), but some are compound:
 * - Tuples: (int, Error) for multiple return values
 * - Error variants: Error, FatalError (same base type, different semantics)
 *
 * Examples:
 *   int                    -> TypeDescriptor(ParamType::Int)
 *   (float, Error)         -> TypeDescriptor({Float, Error})
 *   Error                  -> TypeDescriptor(ParamType::Error, "Error")
 *   FatalError             -> TypeDescriptor(ParamType::Error, "FatalError")
 */
struct TypeDescriptor {
  ParamType base_type;

  std::vector<TypeDescriptor> tuple_elements;
  std::string error_variant;
  std::vector<TypeDescriptor> union_types;

  // For struct types: the name of the user-defined struct
  std::string struct_name;

  // These are concrete TypeDescriptors, one per generic parameter.
  std::vector<TypeDescriptor> type_args;

  // For array types: the element type (e.g. int for Array<int>)
  std::shared_ptr<TypeDescriptor> element_type;

  explicit TypeDescriptor(ParamType t) : base_type(t) {}

  explicit TypeDescriptor(std::vector<TypeDescriptor> elements)
      : base_type(ParamType::Tuple), tuple_elements(std::move(elements)) {}

  TypeDescriptor(ParamType t, std::string variant)
      : base_type(t), error_variant(std::move(variant)) {
    if (!variant.empty() && t != ParamType::Error) {
      throw std::logic_error("Only Error type can have variants");
    }
  }

  // Plain struct (no generics)
  static TypeDescriptor make_struct(std::string name) {
    TypeDescriptor desc(ParamType::Struct);
    desc.struct_name = std::move(name);
    return desc;
  }

  // Builds the monomorphized name "Box<int>" automatically.
  static TypeDescriptor make_generic_struct(std::string base_name,
                                            std::vector<TypeDescriptor> args) {
    TypeDescriptor desc(ParamType::Struct);
    desc.type_args = args;
    // Build canonical monomorphized name: "Box<int,float>"
    std::string mono = base_name + "<";
    for (size_t i = 0; i < args.size(); ++i) {
      if (i > 0)
        mono += ",";
      mono += args[i].to_string();
    }
    mono += ">";
    desc.struct_name = std::move(mono);
    return desc;
  }

  static TypeDescriptor make_array(TypeDescriptor elem) {
    TypeDescriptor desc(ParamType::Array);
    desc.element_type = std::make_shared<TypeDescriptor>(std::move(elem));
    return desc;
  }

  [[nodiscard]] bool is_struct() const {
    return base_type == ParamType::Struct;
  }
  [[nodiscard]] bool is_array() const { return base_type == ParamType::Array; }
  [[nodiscard]] bool is_generic_struct() const {
    return is_struct() && !type_args.empty();
  }

  TypeDescriptor(const TypeDescriptor &) = default;
  TypeDescriptor &operator=(const TypeDescriptor &) = default;

  static TypeDescriptor make_union(std::vector<TypeDescriptor> types) {
    TypeDescriptor desc(ParamType::Union);
    desc.union_types = std::move(types);
    return desc;
  }

  [[nodiscard]] bool is_tuple() const { return base_type == ParamType::Tuple; }
  [[nodiscard]] bool is_error() const { return base_type == ParamType::Error; }
  [[nodiscard]] bool is_nil() const { return base_type == ParamType::Nil; }
  [[nodiscard]] bool is_void() const { return base_type == ParamType::Void; }
  [[nodiscard]] bool is_union() const { return base_type == ParamType::Union; }

  [[nodiscard]] size_t tuple_size() const { return tuple_elements.size(); }

  [[nodiscard]] std::string to_string() const {
    if (is_union()) {
      std::string result = "(";
      for (size_t i = 0; i < union_types.size(); ++i) {
        if (i > 0)
          result += " | ";
        result += union_types[i].to_string();
      }
      result += ")";
      return result;
    }
    if (is_tuple()) {
      std::string result = "(";
      for (size_t i = 0; i < tuple_elements.size(); ++i) {
        if (i > 0)
          result += ", ";
        result += tuple_elements[i].to_string();
      }
      result += ")";
      return result;
    }
    if (is_error() && !error_variant.empty())
      return error_variant;
    if (is_struct())
      return struct_name;
    if (is_array()) {
      return "Array<" + (element_type ? element_type->to_string() : "?") + ">";
    }
    return falcon::atc::to_string(base_type);
  }

  bool operator==(const TypeDescriptor &other) const {
    if (base_type != other.base_type)
      return false;
    if (is_struct())
      return struct_name == other.struct_name;
    if (is_union()) {
      if (union_types.size() != other.union_types.size())
        return false;
      for (size_t i = 0; i < union_types.size(); ++i) {
        if (union_types[i] != other.union_types[i])
          return false;
      }
      return true;
    }
    if (is_tuple()) {
      if (tuple_elements.size() != other.tuple_elements.size())
        return false;
      for (size_t i = 0; i < tuple_elements.size(); ++i) {
        if (!(tuple_elements[i] == other.tuple_elements[i]))
          return false;
      }
      return true;
    }
    if (is_array()) {
      if (!element_type || !other.element_type)
        return element_type == other.element_type;
      return *element_type == *other.element_type;
    }
    return true;
  }

  bool operator!=(const TypeDescriptor &other) const {
    return !(*this == other);
  }
};

// ============================================================================
// EXPRESSIONS (Values - can be nested arbitrarily)
// ============================================================================

/**
 * @brief Base class for all expressions.
 *
 * Expressions represent computations that produce values. They are the
 * building blocks of statements and can be nested arbitrarily (PEMDAS-style).
 *
 * Examples of expression nesting:
 *   (a + b) * c
 *   Config::get_value(gates[i + 1])
 *   read(scope="global", name=prefix + suffix)
 *
 * Every expression has a type that is determined during semantic analysis
 * and stored in `inferred_type` for later use by the interpreter.
 */
class Expr {
public:
  virtual ~Expr() = default;
  virtual std::unique_ptr<Expr> clone() const = 0;

  // Type annotation (filled in during semantic analysis)
  mutable std::optional<TypeDescriptor> inferred_type;

  // Source location (filled in by the parser via set_expr_location).
  // Uses the same 1-indexed convention as Stmt.
  mutable std::string filename;
  mutable int line = 0;
  mutable int column = 0;

protected:
  // Copy location metadata from this node to a freshly-cloned node.
  void copy_location_to(Expr &dest) const {
    dest.filename = filename;
    dest.line = line;
    dest.column = column;
  }
};

/**
 * @brief Literal constant expression.
 *
 * Examples:
 *   42          -> LiteralExpr(int64_t(42))
 *   3.14        -> LiteralExpr(double(3.14))
 *   true        -> LiteralExpr(bool(true))
 *   "hello"     -> LiteralExpr(string("hello"))
 *
 * Why: Literal values are the leaves of expression trees. Using std::variant
 * provides type-safe storage without virtual dispatch overhead.
 */
class LiteralExpr : public Expr {
public:
  std::variant<int64_t, double, bool, std::string> value;

  explicit LiteralExpr(std::variant<int64_t, double, bool, std::string> v)
      : value(std::move(v)) {}

  std::unique_ptr<Expr> clone() const override {
    auto c = std::make_unique<LiteralExpr>(value);
    copy_location_to(*c);
    return c;
  }
};

/**
 * @brief Nil literal (represents absence of error or null value).
 *
 * Example:
 *   Error err = nil;
 *   if (err != nil) { ... }
 *
 * Why: We need a special literal to represent "no error" state. This is
 * distinct from false or 0 - it's the absence of a value. Similar to
 * null in other languages, but only valid for Error type checking.
 *
 * Type checking:
 *   nil == nil           -> always true
 *   error_value != nil   -> true if error occurred
 *   nil can only be compared with Error type
 */
class NilLiteralExpr : public Expr {
public:
  NilLiteralExpr() = default;

  std::unique_ptr<Expr> clone() const override {
    auto c = std::make_unique<NilLiteralExpr>();
    copy_location_to(*c);
    return c;
  }
};

/**
 * @brief Variable reference expression.
 *
 * Examples:
 *   counter              (autotuner-level variable)
 *   success              (state-local variable)
 *   threshold            (input parameter)
 *   plunger_gate         (state input parameter)
 *
 * Why: Variables are the primary way to reference stored values. The
 * semantic analyzer resolves the variable name to a specific scope
 * (autotuner, state-local, parameter) and validates that the variable
 * exists and is in scope.
 *
 * Scope resolution order:
 *   1. State-local variables (highest priority)
 *   2. State input parameters
 *   3. Autotuner-level variables
 *   4. Input parameters (autotuner level)
 *   5. Output parameters (autotuner level - read/write)
 */
class VarExpr : public Expr {
public:
  std::string name;

  // Filled in during semantic analysis
  mutable enum class Scope {
    Unknown,         // Not yet resolved
    StateLocal,      // Declared in current state (destroyed on exit)
    StateInputParam, // State input parameter (read-only in state)
    Autotuner,       // Declared at autotuner level (persistent)
    InputParam,      // Autotuner input parameter (read-only)
    OutputParam // Autotuner output parameter (read/write, refined over time)
  } scope;

  explicit VarExpr(std::string n) : name(std::move(n)), scope(Scope::Unknown) {}

  std::unique_ptr<Expr> clone() const override {
    auto cloned = std::make_unique<VarExpr>(name);
    cloned->scope = scope;
    copy_location_to(*cloned);
    return cloned;
  }
};

/**
 * @brief Binary operation expression.
 *
 * Examples:
 *   a + b                  (arithmetic)
 *   counter * 2            (arithmetic)
 *   x < 10                 (comparison)
 *   err != nil             (error checking)
 *   ready && calibrated    (logical)
 *
 * Why: Binary operations are fundamental to computation. Both operands
 * are expressions, enabling arbitrary nesting like (a + b) * (c - d).
 *
 * Supported operators by category:
 *   Arithmetic: +, -, *, /
 *   Comparison: ==, !=, <, >, <=, >=
 *   Logical:    &&, ||
 *
 * Type rules:
 *   Arithmetic: int/float -> int/float (promotes int to float if mixed)
 *   Comparison: any comparable types -> bool
 *   Logical: bool -> bool
 */
class BinaryExpr : public Expr {
public:
  std::string op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  BinaryExpr(std::string o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
      : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}

  std::unique_ptr<Expr> clone() const override {
    auto c = std::make_unique<BinaryExpr>(op, left->clone(), right->clone());
    copy_location_to(*c);
    return c;
  }
};

/**
 * @brief Unary operation expression.
 *
 * Examples:
 *   -voltage     (numeric negation)
 *   !ready       (logical NOT)
 *
 * Why: Unary operators are common and cleaner than binary equivalents:
 *   -x is clearer than 0 - x
 *   !flag is clearer than flag == false
 *
 * Supported operators:
 *   - : numeric negation (int/float -> int/float)
 *   ! : logical NOT (bool -> bool)
 */
class UnaryExpr : public Expr {
public:
  std::string op; // "-" or "!"
  std::unique_ptr<Expr> operand;

  UnaryExpr(std::string o, std::unique_ptr<Expr> e)
      : op(std::move(o)), operand(std::move(e)) {}

  std::unique_ptr<Expr> clone() const override {
    auto c = std::make_unique<UnaryExpr>(op, operand->clone());
    copy_location_to(*c);
    return c;
  }
};

/**
 * @brief Member access expression (field/method access on structured types).
 *
 * Examples:
 *
 * Field access (property read):
 *   config.plunger_gates               (singleton property)
 *   quantity.value                     (field access)
 *   quantity.unit                      (field access)
 *
 * Method call (function on object):
 *   plunger_gate.name()                (method call - returns string)
 *   connections.size()                 (method call - returns int)
 *
 * Why: While users can't define types, falcon-core types ARE structured
 * with fields and methods. We need both:
 *   - Field access: Read-only property access (no parens)
 *   - Method calls: Functions bound to object (with parens and args)
 *
 * Distinction:
 *   quantity.value      -> Field access (no parens, returns float)
 *   connection.name()   -> Method call (with parens, returns string)
 *
 * This is NOT full OOP:
 *   ✅ Can READ fields: connection.name()
 *   ❌ Can't WRITE fields: connection.name = "new" (compile error)
 *   ❌ Can't define new types
 *   ❌ Can't define methods
 *
 * The semantic analyzer determines if this is:
 *   - Field access: member not followed by ( in parser
 *   - Method call: member followed by ( -> becomes MethodCallExpr
 *
 * Special case: 'config' singleton
 *   config.plunger_gates   -> Returns Connections
 *   config.barrier_gates   -> Returns Connections
 *
 * The 'config' variable is implicitly available in all autotuners.
 */
class MemberExpr : public Expr {
public:
  std::unique_ptr<Expr> object; // Must evaluate to a structured type
  std::string member;           // Field name

  // Set to true during semantic analysis when object is a user-defined struct.
  mutable bool is_struct_access = false;

  MemberExpr(std::unique_ptr<Expr> obj, std::string mem)
      : object(std::move(obj)), member(std::move(mem)) {}

  std::unique_ptr<Expr> clone() const override {
    auto cloned = std::make_unique<MemberExpr>(object->clone(), member);
    cloned->is_struct_access = is_struct_access;
    copy_location_to(*cloned);
    return cloned;
  }
};

/**
 * @brief Method call expression (member function call).
 *
 * Examples:
 *   plunger_gate.name()                 (no args)
 *   connections.size()                  (no args)
 *   connection.set_voltage(3.5)         (with args)
 *
 * Why: This is distinct from MemberExpr because it's a function call
 * that happens to be bound to an object. The object is implicitly the
 * first argument.
 *
 * Semantically:
 *   connection.name()  ≈  Connection::name(connection)
 *
 * Both are valid syntax in our language:
 *   string n1 = plunger_gate.name();              // Method style
 *   string n2 = Connection::name(plunger_gate);   // Qualified call style
 *
 * Method style is more ergonomic for chaining:
 *   gates[0].name()  vs  Connection::name(gates[0])
 */
class MethodCallExpr : public Expr {
public:
  std::unique_ptr<Expr> object; // Object to call method on
  std::string method_name;      // Method name (or struct routine name)
  std::vector<std::unique_ptr<Expr>> args; // Arguments

  // Set to true during semantic analysis when object is a user-defined struct.
  mutable bool is_struct_method = false;

  MethodCallExpr(std::unique_ptr<Expr> obj, std::string method,
                 std::vector<std::unique_ptr<Expr>> a = {})
      : object(std::move(obj)), method_name(std::move(method)),
        args(std::move(a)) {}

  std::unique_ptr<Expr> clone() const override {
    std::vector<std::unique_ptr<Expr>> cloned_args;
    for (const auto &arg : args) {
      cloned_args.push_back(arg->clone());
    }
    auto cloned = std::make_unique<MethodCallExpr>(object->clone(), method_name,
                                                   std::move(cloned_args));
    cloned->is_struct_method = is_struct_method;
    copy_location_to(*cloned);
    return cloned;
  }
};

/**
 * @brief Array/collection indexing expression.
 *
 * Examples:
 *   gates[0]                          (first element)
 *   connections[i]                    (computed index)
 *   Config::get_group(name)[index]    (chained: call then index)
 *
 * Why: Collections (Connections, etc.) need element access. Both object
 * and index are expressions, enabling complex access patterns.
 *
 * Valid on types:
 *   Connections[int] -> Connection
 *   string[int] -> string (single character, though rarely used)
 *
 * Bounds checking:
 *   Runtime error if index < 0 or index >= size
 */
class IndexExpr : public Expr {
public:
  std::unique_ptr<Expr> object; // Must evaluate to indexable type
  std::unique_ptr<Expr> index;  // Must evaluate to int

  IndexExpr(std::unique_ptr<Expr> obj, std::unique_ptr<Expr> idx)
      : object(std::move(obj)), index(std::move(idx)) {}

  std::unique_ptr<Expr> clone() const override {
    auto c = std::make_unique<IndexExpr>(object->clone(), index->clone());
    copy_location_to(*c);
    return c;
  }
};

/**
 * @brief Named argument for function calls.
 *
 * Examples:
 *   read(scope="global", name="counter")
 *   write(scope="state", name="result", value=42)
 *
 * Why: Some builtin functions have many optional parameters. Named
 * arguments make calls more readable and maintainable.
 *
 * Validation:
 *   - Parameter name must match function signature
 *   - Can't mix positional and named args (all or nothing)
 *   - Required parameters must be present
 */
struct CallArg {
  std::optional<std::string> name; // nullopt for positional, set for named
  std::unique_ptr<Expr> value;
  CallArg(std::unique_ptr<Expr> v) : name(std::nullopt), value(std::move(v)) {}
  CallArg(std::string n, std::unique_ptr<Expr> v)
      : name(std::move(n)), value(std::move(v)) {}

  [[nodiscard]] [[nodiscard]] CallArg clone() const {
    if (name.has_value()) {
      return {name.value(), value ? value->clone() : nullptr};
    }
    return {value ? value->clone() : nullptr};
  }

  CallArg(const CallArg &) = delete;
  CallArg(CallArg &&) noexcept = default;
  CallArg &operator=(CallArg &&) noexcept = default;
};

/**
 * @brief Simple function call expression.
 *
 * Examples:
 *   initialize()                  (measurement function, no args)
 *   measure_voltage(gate)         (measurement function, positional args)
 *   InnerAutotuner(x, y)          (autotuner call - composition)
 *   read(scope="globals", name="x")  (builtin with named args)
 *
 * Why: This is for:
 *   1. User-defined measurement functions (implemented in C++)
 *   2. Autotuner composition (calling other autotuners)
 *   3. Builtin functions (without :: qualifier)
 *
 * NOT used for:
 *   - Qualified builtin functions (use QualifiedCallExpr instead)
 *   - Methods (use MethodCallExpr instead)
 */
class CallExpr : public Expr {
public:
  std::string name;
  std::vector<CallArg> arguments;

  CallExpr(std::string n, std::vector<CallArg> args)
      : name(std::move(n)), arguments(std::move(args)) {}

  std::unique_ptr<Expr> clone() const override {
    std::vector<CallArg> cloned_args;
    cloned_args.reserve(arguments.size());
    for (const auto &arg : arguments) {
      cloned_args.push_back(arg.clone());
    }
    auto c = std::make_unique<CallExpr>(name, std::move(cloned_args));
    copy_location_to(*c);
    return c;
  }
};

// ============================================================================
// STATEMENTS (Actions - sequential execution)
// ============================================================================

/**
 * @brief Base class for all statements.
 *
 * Statements represent actions and control flow. They are executed
 * sequentially within a state body.
 *
 * Key insight: Each statement typically corresponds to one line of code
 * ending with a semicolon (though IfStmt spans multiple lines).
 */
class Stmt {
public:
  virtual ~Stmt() = default;
  virtual std::unique_ptr<Stmt> clone() const = 0;

  // Source location for error reporting
  mutable std::string filename;
  mutable int line = 0;
  mutable int column = 0;
};

/**
 * @brief Variable declaration statement.
 *
 * Examples:
 *   int counter = 0;                // With initializer
 *   bool success;                   // Without initializer (default value)
 *   Error err = nil;                // Error initialized to nil
 *   Connections gates = config.plunger_gates;  // Complex type
 *
 * Why: Variables store state at two levels:
 *
 * 1. Autotuner-level (persistent across states):
 *    autotuner MyAutotuner -> (bool success) {
 *      int counter = 0;        <-- Lives for entire autotuner execution
 *      float voltage = 0.0;    <-- Available to all states
 *      ...
 *    }
 *
 * 2. State-level (temporary, destroyed on state exit):
 *    state measure {
 *      float measured = measure_voltage();  <-- Only in this state
 *      bool valid = measured > 0.0;         <-- Destroyed on transition
 *      if (valid) -> process;
 *    }
 *
 * Scoping rules:
 *   - Autotuner-level: Available to all states, persists across transitions
 *   - State-level: Only available in current state, destroyed on exit
 *   - State-level CAN be passed to next state via transition parameter
 *
 * Default values (if no initializer):
 *   int: 0, float: 0.0, bool: false, string: "", Error: nil
 */
class VarDeclStmt : public Stmt {
public:
  TypeDescriptor type;
  std::string name;
  std::optional<std::unique_ptr<Expr>> initializer;

  // Determined during semantic analysis based on where this appears in AST
  mutable enum class DeclScope {
    Unknown,    // Not yet analyzed
    Autotuner,  // Declared at autotuner level (persistent)
    StateLocal, // Declared in state body (temporary)
    StructField // Declared as a field inside a struct definition
  } decl_scope;

  VarDeclStmt(TypeDescriptor t, std::string n,
              std::optional<std::unique_ptr<Expr>> init = std::nullopt)
      : type(std::move(t)), name(std::move(n)), initializer(std::move(init)),
        decl_scope(DeclScope::Unknown) {}

  std::unique_ptr<Stmt> clone() const override {
    auto cloned_init = initializer ? std::make_optional((*initializer)->clone())
                                   : std::nullopt;
    auto cloned =
        std::make_unique<VarDeclStmt>(type, name, std::move(cloned_init));
    cloned->decl_scope = decl_scope;
    return cloned;
  }
};

/**
 * @brief A single assignment target — either a plain variable or a struct
 * field.
 *
 * Examples:
 *   b          -> AssignTarget("b")
 *   q.a_       -> AssignTarget(VarExpr("q"), "a_")
 *
 * Mixed tuple targets:
 *   q.a_, b = method();   ->  [AssignTarget(q,"a_"), AssignTarget("b")]
 */
struct AssignTarget {
  // Plain variable name (used when field_name is empty)
  std::string variable;

  // For struct field targets: the object expression and field name.
  // Both must be set together; if field_name is empty this is a plain var.
  std::unique_ptr<Expr> object; // e.g. VarExpr("q")
  std::string field_name;       // e.g. "a_"

  // Plain variable constructor
  explicit AssignTarget(std::string var) : variable(std::move(var)) {}

  // Struct field constructor
  AssignTarget(std::unique_ptr<Expr> obj, std::string field)
      : object(std::move(obj)), field_name(std::move(field)) {}

  [[nodiscard]] bool is_field_target() const { return !field_name.empty(); }

  AssignTarget(AssignTarget &&) noexcept = default;
  AssignTarget &operator=(AssignTarget &&) noexcept = default;
  AssignTarget(const AssignTarget &) = delete;
  AssignTarget &operator=(const AssignTarget &) = delete;

  [[nodiscard]] AssignTarget clone() const {
    if (is_field_target()) {
      return AssignTarget(object->clone(), field_name);
    }
    return AssignTarget(variable);
  }
};

/**
 * @brief Assignment statement (single or multiple targets).
 *
 * Targets may be plain variables or struct-field accesses, mixed freely:
 *   counter = counter + 1;
 *   result, err = read(...);
 *   q.a_, other = method();   // mixed: struct field + plain var
 */
class AssignStmt : public Stmt {
public:
  std::vector<AssignTarget>
      targets;                 // Assignment targets (variable or obj.field)
  std::unique_ptr<Expr> value; // Expression to evaluate

  AssignStmt(std::vector<AssignTarget> tgts, std::unique_ptr<Expr> val)
      : targets(std::move(tgts)), value(std::move(val)) {}

  std::unique_ptr<Stmt> clone() const override {
    std::vector<AssignTarget> cloned_targets;
    cloned_targets.reserve(targets.size());
    for (const auto &t : targets) {
      cloned_targets.push_back(t.clone());
    }
    return std::make_unique<AssignStmt>(std::move(cloned_targets),
                                        value->clone());
  }

  [[nodiscard]] bool is_tuple_assignment() const { return targets.size() > 1; }

  // Convenience: return target name for single plain-variable assignments
  [[nodiscard]] const std::string &single_target_name() const {
    return targets[0].variable;
  }
};

/**
 * @brief Standalone struct field assignment: q.field = expr;
 *
 * This is the dedicated single-target form. When only one struct field is
 * being assigned (not as part of a tuple), the parser emits this instead of
 * AssignStmt for clarity.
 *
 * Used:
 *   - Inside struct routines with implicit self:  a_ = expr;   -> object =
 * VarExpr("self")
 *   - Outside with explicit object:               q.a_ = expr;
 */
class StructFieldAssignStmt : public Stmt {
public:
  std::unique_ptr<Expr>
      object; // The struct variable (or VarExpr("self") inside a routine)
  std::string field;           // Field name
  std::unique_ptr<Expr> value; // RHS expression

  StructFieldAssignStmt(std::unique_ptr<Expr> obj, std::string f,
                        std::unique_ptr<Expr> val)
      : object(std::move(obj)), field(std::move(f)), value(std::move(val)) {}

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<StructFieldAssignStmt>(object->clone(), field,
                                                   value->clone());
  }
};

/**
 * @brief Expression statement (for side effects).
 *
 * Examples:
 *   log::info("Starting calibration");
 *   write(scope="global", name="result", value=measured);
 *
 * Why: Some expressions are executed for side effects (logging, I/O)
 * rather than their return values.
 */
class ExprStmt : public Stmt {
public:
  std::unique_ptr<Expr> expression;

  explicit ExprStmt(std::unique_ptr<Expr> e) : expression(std::move(e)) {}

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<ExprStmt>(expression->clone());
  }
};

/**
 * @brief Conditional statement (if/else).
 *
 * Examples:
 *   if (err != nil) {
 *     log::error("Operation failed");
 *     -> error_state;
 *   }
 *   else {
 *     -> success_state;
 *   }
 *
 * Why: Conditional logic is fundamental. Each branch can contain
 * any statements including nested ifs and transitions.
 */
class IfStmt : public Stmt {
public:
  std::unique_ptr<Expr> condition;              // Must evaluate to bool
  std::vector<std::unique_ptr<Stmt>> then_body; // Executed if true
  std::vector<std::unique_ptr<Stmt>>
      else_body; // Executed if false (may be empty)

  IfStmt(std::unique_ptr<Expr> cond,
         std::vector<std::unique_ptr<Stmt>> then_stmts,
         std::vector<std::unique_ptr<Stmt>> else_stmts = {})
      : condition(std::move(cond)), then_body(std::move(then_stmts)),
        else_body(std::move(else_stmts)) {}

  std::unique_ptr<Stmt> clone() const override {
    std::vector<std::unique_ptr<Stmt>> cloned_then, cloned_else;
    for (const auto &stmt : then_body)
      cloned_then.push_back(stmt->clone());
    for (const auto &stmt : else_body)
      cloned_else.push_back(stmt->clone());
    return std::make_unique<IfStmt>(condition->clone(), std::move(cloned_then),
                                    std::move(cloned_else));
  }

  bool has_else() const { return !else_body.empty(); }
};

/**
 * @brief State transition statement.
 *
 * Examples:
 *
 * Simple transition:
 *   -> done;
 *
 * Transition with parameter:
 *   -> loop(PlungerGates[counter]);
 *   -> process_gate(current_gate);
 *
 * Why: Transitions change the current state. When executed:
 *   1. Destroy all state-local variables
 *   2. Pass parameter to target state (if provided)
 *   3. Move execution to target state
 *
 * NO cross-autotuner transitions!
 *   Call other autotuners like functions, don't transition to them.
 *
 * Parameter passing:
 *   state init {
 *     Connection gate = gates[0];
 *     -> process(gate);  // Pass gate to next state
 *   }
 *
 *   state process (Connection plunger_gate) {  // Receives as input parameter
 *     // plunger_gate is in scope here (read-only)
 *   }
 */
class TransitionStmt : public Stmt {
public:
  std::string target_state; // Name of state to transition to

  // Parameter to pass to target state (optional)
  // Must match target state's input parameter type
  std::vector<std::unique_ptr<Expr>> parameters;

  TransitionStmt(std::string state,
                 std::vector<std::unique_ptr<Expr>> param = {})
      : target_state(std::move(state)), parameters(std::move(param)) {}

  std::unique_ptr<Stmt> clone() const override {
    std::vector<std::unique_ptr<Expr>> param_clones;
    param_clones.reserve(parameters.size());
    for (const auto &expr : parameters) {
      param_clones.push_back(expr ? expr->clone() : nullptr);
    }
    return std::make_unique<TransitionStmt>(target_state,
                                            std::move(param_clones));
  }

  bool has_parameters() const { return !parameters.empty(); }
};

/**
 * @brief Terminal statement (end of execution).
 *
 * Example:
 *   state done {
 *     success = true;
 *     err = nil;
 *     terminal;
 *   }
 *
 * Why: Marks a state as terminal. When reached, autotuner execution
 * completes successfully and returns output parameters to caller.
 *
 * Validation: Semantic analyzer ensures all output parameters are
 * assigned before any reachable terminal statement.
 */
class TerminalStmt : public Stmt {
public:
  TerminalStmt() = default;

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<TerminalStmt>();
  }
};

// ============================================================================
// PARAMETER DECLARATIONS
// ============================================================================

/**
 * @brief Parameter declaration for autotuner inputs/outputs and state inputs.
 *
 * Used for:
 *   1. Autotuner input parameters:
 *      autotuner MyAutotuner (int threshold, float voltage) -> ...
 *                             ^^^^^^^^^^^^  ^^^^^^^^^^^^^
 *
 *   2. Autotuner output parameters:
 *      autotuner MyAutotuner (...) -> (bool success, Error err)
 *                                      ^^^^^^^^^^^^  ^^^^^^^^^
 *
 *   3. State input parameters:
 *      state process (Connection plunger_gate) {
 *                     ^^^^^^^^^^^^^^^^^^^^^^^^
 *        // plunger_gate is read-only in this state
 *      }
 *
 * Input parameters (autotuner and state):
 *   - Passed when called/transitioned
 *   - Read-only within scope
 *
 * Output parameters (autotuner only):
 *   - Written during execution (can be refined multiple times)
 *   - Returned when autotuner completes
 *   - Must be assigned before terminal
 */
struct ParamDecl {
  TypeDescriptor type;
  std::string name;
  std::optional<std::unique_ptr<Expr>> default_value;

  ParamDecl(TypeDescriptor t, std::string n,
            std::optional<std::unique_ptr<Expr>> def = std::nullopt)
      : type(std::move(t)), name(std::move(n)), default_value(std::move(def)) {}
  std::unique_ptr<ParamDecl> clone() const {
    auto cloned_default = default_value
                              ? std::make_optional((*default_value)->clone())
                              : std::nullopt;
    return std::make_unique<ParamDecl>(type, name, std::move(cloned_default));
  }

  ParamDecl(const ParamDecl &) = delete;
  ParamDecl(ParamDecl &&) noexcept = default;
  ParamDecl &operator=(ParamDecl &&) noexcept = default;
};

// ============================================================================
// STATE AND AUTOTUNER STRUCTURES
// ============================================================================

/**
 * @brief State declaration.
 *
 * Examples:
 *
 * Simple state (no input parameter):
 *   state init {
 *     log::info("Initializing");
 *     -> process;
 *   }
 *
 * State with input parameter (receives value from transition):
 *   state loop (Connection plunger_gate) {
 *     log::info("Processing gate: {}", plunger_gate.name());
 *     if (counter < max_iterations) {
 *       counter = counter + 1;
 *       -> loop(PlungerGates[counter]);
 *     }
 *     else {
 *       -> done;
 *     }
 *   }
 *
 * Why: States are the fundamental execution unit. Each state:
 *   - Has a name (for transitions)
 *   - Optionally accepts an input parameter (passed via transition)
 *   - Contains a body of statements executed sequentially
 *
 * Execution model:
 *   1. Enter state with parameter (if provided)
 *   2. Execute statements in body sequentially
 *   3. When TransitionStmt is hit, destroy state-local vars and move to target
 *   4. When TerminalStmt is hit, end autotuner execution
 *
 * Variable scoping:
 *   - Can access: autotuner vars (read/write), input params (read-only),
 *     output params (read/write), state input param (read-only)
 *   - Can declare: state-local variables (temporary, destroyed on exit)
 *
 * State input parameters replace generics:
 *   Old (with generics):  state loop [gate] { ... }
 *   New (with params):    state loop (Connection gate) { ... }
 *
 * This is clearer because:
 *   - Type is explicit
 *   - Looks like function parameters (familiar)
 *   - Can be validated by type checker
 */
struct StateDecl {
  std::string name;

  // Input parameter (optional) - replaces old generic parameter
  // Passed when transitioning to this state
  // Example: state loop (Connection plunger_gate)
  std::vector<std::unique_ptr<ParamDecl>> input_parameters;

  // State body: sequence of statements executed on entry
  std::vector<std::unique_ptr<Stmt>> body;

  StateDecl(std::string n,
            std::vector<std::unique_ptr<ParamDecl>> input_param = {},
            std::vector<std::unique_ptr<Stmt>> b = {})
      : name(std::move(n)), input_parameters(std::move(input_param)),
        body(std::move(b)) {}

  StateDecl(StateDecl &&) noexcept = default;
  StateDecl &operator=(StateDecl &&) noexcept = default;
  StateDecl(const StateDecl &) = delete;
  StateDecl &operator=(const StateDecl &) = delete;

  [[nodiscard]] bool has_input_parameters() const {
    return !input_parameters.empty();
  }
};

/**
 * @brief Complete autotuner declaration.
 *
 * Full example:
 *   autotuner GenericIterationTest (Gname gname) -> (int counter) {
 *     int counter = 0;
 *     Connections PlungerGates = Config::get_group_plunger_gates(gname);
 *     Connection PlungerGate = PlungerGates[0];
 *     int max_iterations = PlungerGates.size();
 *
 *     start -> loop(PlungerGate);
 *
 *     state loop (Connection plunger_gate) {
 *       log::info("The current plunger-gate is {}", plunger_gate.name());
 *       if (counter < max_iterations) {
 *         counter = counter + 1;
 *         -> loop(PlungerGates[counter]);
 *       }
 *       else {
 *         -> done;
 *       }
 *     }
 *
 *     state done {
 *       terminal;
 *     }
 *   }
 *
 * Components:
 *
 * 1. Name: Identifier for calling this autotuner
 *
 * 2. Input parameters: Values passed when called
 *    - Read-only within autotuner
 *    - Can have defaults
 *
 * 3. Output parameters: Values returned when complete
 *    - Read/write (can be refined multiple times)
 *    - Must be assigned before terminal
 *
 * 4. Required autotuners: Dependencies
 *    - Listed in requires clause
 *    - Can be called like functions
 *
 * 5. Autotuner-level variables: Persistent state
 *    - Declared at top level
 *    - Available to all states (read/write)
 *    - Persist across state transitions
 *
 * 6. Entry state: Where execution begins
 *    - Specified by "start -> state_name;" or "start -> state_name(param);"
 *
 * 7. States: The state machine
 *    - Named code blocks
 *    - Can accept input parameters via transitions
 *    - Execute sequentially until transition or terminal
 *
 * Symbol table structure:
 *   ┌─────────────────────────────────────┐
 *   │ AutotunerDecl                        │
 *   ├─────────────────────────────────────┤
 *   │ input_params (read-only)            │  Accessible
 *   │ output_params (read/write)          │  to all
 *   │ autotuner_variables (read/write)    │  states
 *   ├─────────────────────────────────────┤
 *   │ StateDecl                            │
 *   │ ├─ input_parameter (read-only)      │  State-
 *   │ └─ body                              │  specific
 *   │    └─ VarDeclStmt (state-local)     │  (temp)
 *   └─────────────────────────────────────┘
 */
struct AutotunerDecl {
  std::string name;
  std::string module_name; // Set after parsing

  // Interface
  std::vector<std::unique_ptr<ParamDecl>>
      input_params; // Read-only in autotuner
  std::vector<std::unique_ptr<ParamDecl>>
      output_params; // Read/write (can refine multiple times)

  // Persistent variables (autotuner scope - available to all states)
  std::vector<std::unique_ptr<Stmt>> autotuner_variables;

  // Entry point (which state to execute first, optionally with parameters)
  std::string entry_state;
  std::vector<std::unique_ptr<Expr>>
      entry_parameters; // For: start -> loop(gates[0], some_value);

  // State machine
  std::vector<StateDecl> states;

  AutotunerDecl(std::string n, std::vector<std::unique_ptr<ParamDecl>> inputs,
                std::vector<std::unique_ptr<ParamDecl>> outputs,
                std::vector<std::unique_ptr<Stmt>> vars, std::string entry,
                std::vector<std::unique_ptr<Expr>> entry_params,
                std::vector<StateDecl> sts)
      : name(std::move(n)), input_params(std::move(inputs)),
        output_params(std::move(outputs)), autotuner_variables(std::move(vars)),
        entry_state(std::move(entry)),
        entry_parameters(std::move(entry_params)), states(std::move(sts)) {}

  AutotunerDecl(AutotunerDecl &&) noexcept = default;
  AutotunerDecl &operator=(AutotunerDecl &&) noexcept = default;
  AutotunerDecl(const AutotunerDecl &) = delete;
  AutotunerDecl &operator=(const AutotunerDecl &) = delete;
};

// ============================================================================
// STRUCT DECLARATIONS
// ============================================================================

/**
 * @brief A routine declared inside a struct body.
 *
 * Struct routines are like RoutineDecl but are defined (have a body), and
 * inside the body bare field names (a_, b_) implicitly refer to the struct's
 * own fields via a synthetic "self" variable.
 *
 * Operator overloads use reserved names:
 *   "Equal"       -> ==
 *   "NotEqual"    -> !=
 *   "Add"         -> +
 *   "Subtract"    -> -
 *   "LessThan"    -> <
 *   "GreaterThan" -> >
 *   "Multiply"    -> *
 *   "Divide"      -> /
 */
struct RoutineDecl {
  std::string name;
  std::string module_name; // Set after parsing
  std::vector<std::unique_ptr<ParamDecl>> input_params;
  std::vector<std::unique_ptr<ParamDecl>> output_params;
  std::vector<std::unique_ptr<Stmt>> body;

  RoutineDecl(std::string n, std::vector<std::unique_ptr<ParamDecl>> inputs,
              std::vector<std::unique_ptr<ParamDecl>> outputs,
              std::vector<std::unique_ptr<Stmt>> b = {})
      : name(std::move(n)), input_params(std::move(inputs)),
        output_params(std::move(outputs)), body(std::move(b)) {}

  RoutineDecl(RoutineDecl &&) noexcept = default;
  RoutineDecl &operator=(RoutineDecl &&) noexcept = default;
  RoutineDecl(const RoutineDecl &) = delete;
  RoutineDecl &operator=(const RoutineDecl &) = delete;

  // Returns the operator symbol this routine overloads, or "" if none.
  [[nodiscard]] std::string operator_symbol() const {
    if (name == "Equal") {
      return "==";
    }
    if (name == "NotEqual") {
      return "!=";
    }
    if (name == "Add") {
      return "+";
    }
    if (name == "Subtract") {
      return "-";
    }
    if (name == "LessThan") {
      return "<";
    }
    if (name == "GreaterThan") {
      return ">";
    }
    if (name == "Multiply") {
      return "*";
    }
    if (name == "Divide") {
      return "/";
    }
    return "";
  }
};

/**
 * A struct declaration, optionally parameterised by type variables.
 *
 *   struct Box <T> {
 *       T value;
 *       routine New (T v) -> (Box<T> b) { b.value = v; }
 *       routine Get  -> (T v)           { v = this.value; }
 *   }
 *
 * `generic_params` holds the declared parameter names in order: ["T"].
 * Non-generic structs have an empty `generic_params` vector (backward
 * compatible — all existing code continues to work unchanged).
 *
 * Monomorphization strategy (interpreter-level, NOT compile-time):
 *   When the interpreter sees  Box<int> x = Box.New(42);  it:
 *     1. Resolves the base struct declaration for "Box".
 *     2. Builds a substitution map  { "T" -> TypeDescriptor(Int) }.
 *     3. Looks up (or creates on demand) a monomorphized StructDecl stored
 *        under the key "Box<int>" in the TypeRegistry.
 *     4. The monomorphized decl has all field/param types substituted.
 */
struct StructDecl {
  std::string name;        // The type name, e.g. "Box"
  std::string module_name; // Set after parsing

  // Empty for non-generic structs (fully backward compatible).
  std::vector<std::string> generic_params;

  // Fields stored as VarDeclStmt (decl_scope = StructField).
  std::vector<VarDeclStmt> fields;
  std::vector<RoutineDecl> routines;

  // Plain constructor (non-generic, or used internally for monomorphized
  // copies)
  StructDecl(std::string n, std::vector<VarDeclStmt> fs,
             std::vector<RoutineDecl> rs)
      : name(std::move(n)), fields(std::move(fs)), routines(std::move(rs)) {}

  // ── NEW: constructor that also takes generic params
  StructDecl(std::string n, std::vector<std::string> gp,
             std::vector<VarDeclStmt> fs, std::vector<RoutineDecl> rs)
      : name(std::move(n)), generic_params(std::move(gp)),
        fields(std::move(fs)), routines(std::move(rs)) {}

  StructDecl(StructDecl &&) noexcept = default;
  StructDecl &operator=(StructDecl &&) noexcept = default;
  StructDecl(const StructDecl &) = delete;
  StructDecl &operator=(const StructDecl &) = delete;

  [[nodiscard]] bool is_generic() const { return !generic_params.empty(); }

  [[nodiscard]] const VarDeclStmt *find_field(const std::string &n) const {
    for (const auto &f : fields) {
      if (f.name == n)
        return &f;
    }
    return nullptr;
  }

  [[nodiscard]] const RoutineDecl *find_routine(const std::string &n) const {
    for (const auto &r : routines) {
      if (r.name == n)
        return &r;
    }
    return nullptr;
  }

  [[nodiscard]] const RoutineDecl *find_operator(const std::string &op) const {
    for (const auto &r : routines) {
      if (r.operator_symbol() == op)
        return &r;
    }
    return nullptr;
  }
};

struct FFImportDecl {
  std::string wrapper_file;
  std::vector<std::string> imports;
  std::vector<std::string> build_libs;

  FFImportDecl(std::string wrapper, std::vector<std::string> imps = {},
               std::vector<std::string> libs = {})
      : wrapper_file(std::move(wrapper)), imports(std::move(imps)),
        build_libs(std::move(libs)) {}
};
/**
 * @brief Complete program (collection of autotuners and routines).
 *
 * A .fal file can contain:
 * - Multiple autotuner definitions
 * - Multiple routine declarations
 * - All of the imports for the project
 */
struct Program {
  std::string module_name; // Set after parsing
  std::vector<StructDecl>
      structs; // User-defined struct types (parsed before autotuners)
  std::vector<AutotunerDecl> autotuners;
  std::vector<RoutineDecl> routines;
  std::vector<std::string> imports;
  std::vector<FFImportDecl> ff_imports;

  // Fast lookup indexes (call build_indexes() after parsing)
  mutable std::map<std::string, const AutotunerDecl *> autotuner_index;
  mutable std::map<std::string, const RoutineDecl *> routine_index;
  mutable std::map<std::string, const StructDecl *> struct_index;
  mutable std::map<std::string, const FFImportDecl *> ff_import_index;

  void build_indexes() const {
    struct_index.clear();
    autotuner_index.clear();
    routine_index.clear();
    ff_import_index.clear();
    for (const auto &s : structs) {
      struct_index[s.name] = &s;
    }
    for (const auto &a : autotuners) {
      autotuner_index[a.name] = &a;
    }
    for (const auto &r : routines) {
      routine_index[r.name] = &r;
    }
    for (const auto &ffi : ff_imports) {
      ff_import_index[ffi.wrapper_file] = &ffi;
    }
  }

  [[nodiscard]] bool is_struct_type(const std::string &name) const {
    return struct_index.count(name) > 0;
  }
};

// ============================================================================
// BUILTIN FUNCTION REGISTRY (for parse-time validation)
// ============================================================================

enum class NamedArgs : uint8_t { No, Uses };
/**
 * @brief Signature of a builtin function for validation.
 *
 * The parser validates builtin function calls against this registry
 * at parse time, catching errors early.
 */
struct BuiltinSignature {
  std::string qualified_name; // "Config::get_group_plunger_gates"

  // Parameter specification
  struct ParamSpec {
    std::string name; // For named parameters
    TypeDescriptor type;
    bool required;

    ParamSpec(std::string n, TypeDescriptor t, bool req = true)
        : name(std::move(n)), type(std::move(t)), required(req) {}
    [[nodiscard]] nlohmann::json to_json() const {
      return {
          {"name", name}, {"type", type.to_string()}, {"required", required}};
    }
    ParamSpec(const ParamSpec &) = default;
    ParamSpec &operator=(const ParamSpec &) = default;
  };

  std::vector<ParamSpec> parameters;
  std::vector<ParamSpec> return_params;

  // Does this function support named arguments?
  bool supports_named_args;

  BuiltinSignature(std::string name, std::vector<ParamSpec> params,
                   std::vector<ParamSpec> returns,
                   NamedArgs named_args = NamedArgs::No)
      : qualified_name(std::move(name)), parameters(std::move(params)),
        return_params(std::move(returns)),
        supports_named_args(named_args == NamedArgs::Uses) {}

  BuiltinSignature(const BuiltinSignature &) = default;
  BuiltinSignature &operator=(const BuiltinSignature &) = default;
  [[nodiscard]] nlohmann::json to_json() const {
    nlohmann::json j;
    j["qualified_name"] = qualified_name;
    j["supports_named_args"] = supports_named_args;
    j["parameters"] = nlohmann::json::array();
    for (const auto &p : parameters)
      j["parameters"].push_back(p.to_json());
    j["return_params"] = nlohmann::json::array();
    for (const auto &p : return_params)
      j["return_params"].push_back(p.to_json());
    return j;
  }

  [[nodiscard]] std::vector<std::string> get_return_names() const {
    std::vector<std::string> names;
    names.reserve(return_params.size());
    for (const auto &param : return_params) {
      names.push_back(param.name);
    }
    return names;
  }

  [[nodiscard]] std::vector<std::string> get_param_names() const {
    std::vector<std::string> names;
    names.reserve(parameters.size());
    for (const auto &param : parameters) {
      names.push_back(param.name);
    }
    return names;
  }
};

/**
 * @brief Registry of all builtin functions.
 *
 * The parser queries this during semantic analysis to validate
 * builtin function calls before code generation.
 */
class BuiltinFunctionRegistry {
public:
  void register_builtin(const BuiltinSignature &sig) {
    // Use insert instead of operator[] to avoid default construction
    builtins_.insert({sig.qualified_name, sig});
    // OR use emplace:
    // builtins_.emplace(sig.qualified_name, sig);
  }

  [[nodiscard]] const BuiltinSignature *
  lookup(const std::string &qualified_name) const {
    auto it = builtins_.find(qualified_name);
    return it != builtins_.end() ? &it->second : nullptr;
  }

  [[nodiscard]] bool exists(const std::string &qualified_name) const {
    return builtins_.find(qualified_name) != builtins_.end();
  }

  static BuiltinFunctionRegistry create_default();

private:
  std::map<std::string, BuiltinSignature> builtins_;
};

} // namespace falcon::atc
