%require "3.8"
%language "c++"

%define api.namespace {falcon::atc}
%define api.parser.class {Parser}
%define api.value.type variant
%define api.token.constructor
%define api.token.prefix {TOK_}
%define parse.error verbose
%define parse.trace
%locations

%code requires {
  #include <string>
  #include <vector>
  #include <memory>
  #include <optional>
  #include <set>
  #include "falcon-atc/AST.hpp"
  #include "falcon-atc/ParseError.hpp"
}

%code {
  #include <iostream>
  
  namespace falcon::atc {
    Parser::symbol_type yylex();
  }
  
  std::unique_ptr<falcon::atc::Program> program_root;
  std::vector<falcon::atc::ParseError> current_errors;
  
  // Scope tracking for variable declarations
  std::set<std::string> autotuner_scope;      // Autotuner-level variables
  std::set<std::string> autotuner_input_params;   // Input parameters (read-only)
  std::set<std::string> autotuner_output_params;  // Output parameters (read/write)
  std::set<std::string> state_local_scope;    // State-local variables
  std::set<std::string> state_input_params;    // Current state's input parameter
  bool parsing_routine_params = false;
  std::set<std::string> routine_input_params;
  
  void clear_autotuner_scope() {
    autotuner_scope.clear();
    autotuner_input_params.clear();
    autotuner_output_params.clear();
    state_local_scope.clear();
    state_input_params.clear();
  }
  
  void clear_state_scope() {
    state_local_scope.clear();
    state_input_params.clear();
  }  

  // -----------------------------------------------------------------------
  // Struct context tracking
  // -----------------------------------------------------------------------

  // Names of all struct types declared so far in this file.
  // Used by type_spec to allow struct names as types.
  std::set<std::string> struct_known_types;

  // The set of field names belonging to the struct currently being parsed.
  // Populated while parsing struct_field_list, cleared after each struct_decl.
  std::set<std::string> struct_field_scope;

  // True while we are inside a struct routine body.
  // When true, bare IDENTIFIER = expr is checked against struct_field_scope
  // first (becomes a StructFieldAssignStmt targeting "self").
  bool in_struct_routine = false;

  void enter_struct_routine() {
    in_struct_routine = true;
    // struct routine has its own mini-scope for input/output params
    autotuner_input_params.clear();
    autotuner_output_params.clear();
    state_local_scope.clear();
    state_input_params.clear();
  }

  void leave_struct_routine() {
    in_struct_routine = false;
    autotuner_input_params.clear();
    autotuner_output_params.clear();
    state_local_scope.clear();
    state_input_params.clear();
  }
  
  bool is_variable_declared(const std::string& name) {
    // When inside a struct routine, bare field names are implicitly in scope
    if (in_struct_routine && struct_field_scope.count(name) > 0) return true;
    return autotuner_scope.count(name) > 0 ||
           autotuner_input_params.count(name) > 0 ||
           autotuner_output_params.count(name) > 0 ||
           state_local_scope.count(name) > 0 ||
           state_input_params.count(name) > 0;
  }
  
  bool is_redeclaration(const std::string& name, bool in_state) {
    if (in_state) {
      // In state: can't redeclare autotuner-level vars, input params, output params, or state input param
      return autotuner_scope.count(name) > 0 ||
             autotuner_input_params.count(name) > 0 ||
             autotuner_output_params.count(name) > 0 ||
             state_input_params.count(name) > 0 ||
             state_local_scope.count(name) > 0;
    } else {
      // At autotuner level: can't redeclare autotuner vars, input params, or output params
      return autotuner_scope.count(name) > 0 ||
             autotuner_input_params.count(name) > 0 ||
             autotuner_output_params.count(name) > 0;
    }
  }

  void set_stmt_location(falcon::atc::Stmt* stmt, const falcon::atc::Parser::location_type& loc) {
    if (stmt) {
      stmt->filename = falcon::atc::current_filename;
      stmt->line = loc.begin.line;
      stmt->column = loc.begin.column;
    }
  }
}

// Token declarations
%token <std::string> IDENTIFIER DOUBLE INTEGER STRING

%token AUTOTUNER ROUTINE STATE STRUCT IMPORT START USES TERMINAL IF ELIF ELSE TRUE FALSE NIL 
%token FLOAT_KW INT_KW BOOL_KW STRING_KW ERROR_KW
%token ARROW LBRACKET RBRACKET LBRACE RBRACE LPAREN RPAREN ASSIGN COMMA SEMICOLON DOT
%token PLUS MINUS MUL DIV EQ NE LL GG LE GE AND OR NOT

// Type declarations for grammar rules
%type <std::unique_ptr<Program>> program
%type <std::vector<AutotunerDecl>> autotuner_list
%type <std::unique_ptr<AutotunerDecl>> autotuner_decl
%type <std::vector<std::unique_ptr<ParamDecl>>> param_decl_list input_params output_params state_input_params
%type <std::unique_ptr<ParamDecl>> param_decl
%type <std::unique_ptr<TypeDescriptor>> type_spec
%type <std::vector<std::string>> requires_clause identifier_list import_list import_stmt import_string_list
%type <std::vector<std::unique_ptr<Stmt>>> autotuner_var_decls routine_body routine_body_stmts 
%type <std::unique_ptr<VarDeclStmt>> var_decl_stmt struct_field_decl
%type <std::string> entry_state 
%type <std::vector<std::unique_ptr<Expr>>> entry_params
%type <std::vector<StateDecl>> state_list
%type <std::unique_ptr<StateDecl>> state_decl
%type <std::vector<std::unique_ptr<Stmt>>> stmt_list elif_chain
%type <std::unique_ptr<Stmt>> stmt struct_routine_stmt
%type <std::unique_ptr<Expr>> expr primary_expr postfix_expr
%type <std::vector<std::unique_ptr<Expr>>> expr_list 
%type <std::vector<CallArg>> call_arg_list 
%type <std::unique_ptr<CallArg>> call_arg
%type <std::vector<RoutineDecl>> routine_list
%type <std::unique_ptr<RoutineDecl>> routine_decl
%type <std::vector<StructDecl>>        struct_decl_list
%type <std::unique_ptr<StructDecl>>    struct_decl
%type <std::vector<VarDeclStmt>>       struct_field_list
%type <std::vector<RoutineDecl>> struct_routine_list
%type <std::vector<AssignTarget>>      assign_target_list
%type <std::unique_ptr<AssignTarget>> assign_target
%type <std::vector<std::variant<
    std::unique_ptr<StructDecl>,
    std::unique_ptr<RoutineDecl>,
    std::unique_ptr<AutotunerDecl>
>> > program_items

%type <std::variant<
    std::unique_ptr<StructDecl>,
    std::unique_ptr<RoutineDecl>,
    std::unique_ptr<AutotunerDecl>
>> program_item

%left OR
%left AND
%left EQ NE 
%left LL GG LE GE
%left PLUS MINUS
%left MUL DIV
%right NOT UMINUS
%left DOT LBRACKET LPAREN

%start program

%%

// ============================================================================
// PROGRAM STRUCTURE
// ============================================================================

program[result]
    : import_list[imps] program_items[items]
      {
        auto prog = std::make_unique<Program>();
        prog->imports = std::move($imps);
        for (auto& item : $items) {
          if (std::holds_alternative<std::unique_ptr<StructDecl>>(item)) {
            prog->structs.push_back(std::move(*std::get<std::unique_ptr<StructDecl>>(item)));
          } else if (std::holds_alternative<std::unique_ptr<RoutineDecl>>(item)) {
            prog->routines.push_back(std::move(*std::get<std::unique_ptr<RoutineDecl>>(item)));
          } else if (std::holds_alternative<std::unique_ptr<AutotunerDecl>>(item)) {
            prog->autotuners.push_back(std::move(*std::get<std::unique_ptr<AutotunerDecl>>(item)));
          }
        }
        $result = std::move(prog);
        program_root = std::move($result);
      }
    ;

program_items[result]
    : %empty
      { $result = std::vector<std::variant<
          std::unique_ptr<StructDecl>,
          std::unique_ptr<RoutineDecl>,
          std::unique_ptr<AutotunerDecl>
        >>(); }
    | program_items[prev] program_item[next]
      {
        $result = std::move($prev);
        $result.push_back(std::move($next));
      }
    ;

program_item[result]
    : struct_decl[item] { $result = std::move($item); }
    | routine_decl[item] { $result = std::move($item); }
    | autotuner_decl[item] { $result = std::move($item); }
    ;

import_list[result]
    : /* empty */ 
       %empty { $result = std::vector<std::string>(); }
    | import_list[existing] import_stmt[next] 
      { 
        $result = std::move($existing);
        $result.insert($result.end(), std::make_move_iterator($next.begin()), std::make_move_iterator($next.end()));
      }
    ;

autotuner_list[result]
    : %empty
      {
        $result = std::vector<AutotunerDecl>();
      }
    | autotuner_list[existing_list] autotuner_decl[next_autotuner]
      {
        $result = std::move($existing_list);
        $result.push_back(std::move(*$next_autotuner));
      }
    ;

routine_list[result]
    : %empty
      {
        $result = std::vector<RoutineDecl>();
      }
    | routine_list[existing_routines] routine_decl[next_routine]
      {
        $result = std::move($existing_routines);
        $result.push_back(std::move(*$next_routine));
      }
    ;

// ============================================================================
// STRUCT DECLARATIONS
// ============================================================================

struct_decl_list[result]
    : %empty
      { $result = std::vector<StructDecl>(); }
    | struct_decl_list[prev] struct_decl[next]
      {
        $result = std::move($prev);
        $result.push_back(std::move(*$next));
      }
    ;

struct_decl[result]
    : STRUCT IDENTIFIER[name] LBRACE
      {
        // Clear field scope for this new struct
        struct_field_scope.clear();
      }
      struct_field_list[fields] struct_routine_list[routines] RBRACE
      {
        // Register the struct name so type_spec can use it from this point on
        struct_known_types.insert($name);
        struct_field_scope.clear();
        $result = std::make_unique<StructDecl>(
            std::move($name),
            std::move($fields),
            std::move($routines));
      }
    ;

// ---------------------------------------------------------------------------
// Struct field declarations  (reuses VarDeclStmt with DeclScope::StructField)
// ---------------------------------------------------------------------------

struct_field_list[result]
    : %empty
      { $result = std::vector<VarDeclStmt>(); }
    | struct_field_list[prev] struct_field_decl[next]
      {
        $result = std::move($prev);
        $result.push_back(std::move(*$next));
      }
    ;

struct_field_decl[result]
    : type_spec[type] IDENTIFIER[name] SEMICOLON
      {
        struct_field_scope.insert($name);
        auto decl = std::make_unique<VarDeclStmt>(
            std::move(*$type), std::move($name), std::nullopt);
        decl->decl_scope = VarDeclStmt::DeclScope::StructField;
        $result = std::move(decl);
      }
    | type_spec[type] IDENTIFIER[name] ASSIGN expr[default_val] SEMICOLON
      {
        struct_field_scope.insert($name);
        auto decl = std::make_unique<VarDeclStmt>(
            std::move(*$type), std::move($name),
            std::make_optional(std::move($default_val)));
        decl->decl_scope = VarDeclStmt::DeclScope::StructField;
        $result = std::move(decl);
      }
    ;

// ---------------------------------------------------------------------------
// Struct routine declarations
// ---------------------------------------------------------------------------

struct_routine_list[result]
    : %empty
      { $result = std::vector<RoutineDecl>(); }
    | struct_routine_list[prev] 
      {
        enter_struct_routine();
      }
      routine_decl[next]
      {
        leave_struct_routine();
        $result = std::move($prev);
        $result.push_back(std::move(*$next));
      }
    ;

// routine Name (inputs) -> (outputs) { body }
routine_decl[result]
    : ROUTINE IDENTIFIER[name]
      {
        parsing_routine_params = true;
      }
      input_params[inputs] ARROW output_params[outputs] routine_body[body]
      {
        parsing_routine_params = false;
        $result = std::make_unique<RoutineDecl>(
            std::move($name),
            std::move($inputs),
            std::move($outputs),
            std::move($body));
      }
    ;

// ---------------------------------------------------------------------------
// Body of a struct routine (statements that can access struct fields)
// ---------------------------------------------------------------------------

routine_body[result]
    : LBRACE routine_body_stmts[stmts] RBRACE
      { $result = std::move($stmts); }
    | %empty
      { $result = std::vector<std::unique_ptr<Stmt>>(); }
    ;

routine_body_stmts[result]
    : %empty
      { $result = std::vector<std::unique_ptr<Stmt>>(); }
    | routine_body_stmts[prev] struct_routine_stmt[next]
      {
        $result = std::move($prev);
        $result.push_back(std::move($next));
      }
    ;

struct_routine_stmt[result]
    // Variable declaration (e.g. Quantity q; or int x = 0;)
    : var_decl_stmt[vd]
      {
        $result = std::move($vd);
        set_stmt_location($result.get(), @vd);
      }
    // Bare field assignment: a_ = expr;
    // If the name is a known struct field, emit StructFieldAssignStmt(self, field, val).
    // If it's an output/local param, emit a plain single-target AssignStmt.
    | IDENTIFIER[name] ASSIGN expr[val] SEMICOLON
      {
        if (struct_field_scope.count($name) > 0) {
          // Implicit self.field = val
          $result = std::make_unique<StructFieldAssignStmt>(
              std::make_unique<VarExpr>("self"),
              std::move($name),
              std::move($val));
        } else if (is_variable_declared($name)) {
          if (autotuner_input_params.count($name) > 0 ||
              state_input_params.count($name) > 0) {
            error(@name, "Cannot assign to read-only parameter: " + $name);
            YYABORT;
          }
          std::vector<AssignTarget> targets;
          targets.emplace_back($name);
          $result = std::make_unique<AssignStmt>(
              std::move(targets), std::move($val));
        } else {
          error(@name, "Undeclared variable in struct routine: " + $name);
          YYABORT;
        }
        set_stmt_location($result.get(), @name);
      }
    // Dot-field assignment: q.field = expr;
    | expr[object] DOT IDENTIFIER[field] ASSIGN expr[val] SEMICOLON
      {
        $result = std::make_unique<StructFieldAssignStmt>(
            std::move($object),
            std::move($field),
            std::move($val));
        set_stmt_location($result.get(), @object);
      }
    // if statement (reuse same form as normal stmt)
    | IF LPAREN expr[cond] RPAREN LBRACE routine_body[then_b] RBRACE elif_chain[else_b]
      {
        $result = std::make_unique<IfStmt>(
            std::move($cond),
            std::move($then_b),
            std::move($else_b));
        set_stmt_location($result.get(), @IF);
      }
    // Expression statement (side effects, e.g. logInfo(...))
    | expr[e] SEMICOLON
      {
        $result = std::make_unique<ExprStmt>(std::move($e));
        set_stmt_location($result.get(), @e);
      }
    ;;

// ============================================================================
// IMPORT DECLARATION
// ============================================================================

import_stmt[result]
    : IMPORT STRING[path] SEMICOLON 
      { $result = std::vector<std::string>{std::move($path)}; }
    | IMPORT LPAREN import_string_list[paths] RPAREN 
      { $result = std::move($paths); }
    ;

import_string_list[result]
    : STRING[first] 
      { $result = std::vector<std::string>{std::move($first)}; }
    | import_string_list[existing] STRING[next] 
      { 
        $result = std::move($existing); 
        $result.push_back(std::move($next)); 
      }
    ;


// ============================================================================
// AUTOTUNER DECLARATION
// ============================================================================

autotuner_decl[result]
    : AUTOTUNER IDENTIFIER[name] 
      {
        // Clear scope when starting new autotuner
        clear_autotuner_scope();
      }
      input_params[inputs] 
      ARROW 
      output_params[outputs] 
      LBRACE
        requires_clause[uses]
        autotuner_var_decls[vars]
        entry_state[entry] entry_params[params] SEMICOLON
        state_list[states]
      RBRACE
      {
        $result = std::make_unique<AutotunerDecl>(
          std::move($name),
          std::move($inputs),
          std::move($outputs),
          std::move($uses),
          std::move($vars),
          std::move($entry),
          std::move($params),
          std::move($states)
        );
      }
    ;

// ============================================================================
// PARAMETERS
// ============================================================================

input_params[result]
    : LPAREN param_decl_list[params] RPAREN
      {
        std::set<std::string>& param_set = parsing_routine_params ? routine_input_params : autotuner_input_params;
        param_set.clear();
        for (const auto& param : $params) {
          if (param_set.count(param->name) > 0) {
            error(@params, "Duplicate input parameter: " + param->name);
            YYABORT;
          }
          param_set.insert(param->name);
        }
        $result = std::move($params);
      }
    | %empty
      {
        std::set<std::string>& param_set = parsing_routine_params ? routine_input_params : autotuner_input_params;
        param_set.clear();
        $result = std::vector<std::unique_ptr<ParamDecl>>();
      }
    ;

output_params[result]
    : LPAREN param_decl_list[params] RPAREN
      {
        // Register output parameters in scope (read/write)
        for (const auto& param : $params) {
          if (autotuner_output_params.count(param->name) > 0) {
            error(@params, "Duplicate output parameter: " + param->name);
            YYABORT;
          }
          if (autotuner_input_params.count(param->name) > 0) {
            error(@params, "Output parameter '" + param->name + "' conflicts with input parameter");
            YYABORT;
          }
          autotuner_output_params.insert(param->name);
        }
        $result = std::move($params);
      }
    | %empty
      { 
        $result = std::vector<std::unique_ptr<ParamDecl>>(); 
      }
    ;

param_decl_list[result]
    : %empty
      { $result = std::vector<std::unique_ptr<ParamDecl>>(); }
    | param_decl_list[existing_params] COMMA param_decl[next_param]
      {
        $result = std::move($existing_params);
        $result.push_back(std::move($next_param));
      }
    | param_decl[first_param]
      {
        $result = std::vector<std::unique_ptr<ParamDecl>>();
        $result.push_back(std::move($first_param));
      }
    ;

param_decl[result]
    : type_spec[type] IDENTIFIER[name]
      {
        $result = std::make_unique<ParamDecl>(std::move(*$type), std::move($name));
      }
    | type_spec[type] IDENTIFIER[name] ASSIGN expr[default_value]
      {
        $result = std::make_unique<ParamDecl>(
          std::move(*$type), 
          std::move($name), 
          std::make_optional(std::move($default_value))
        );
      }
    ;

state_input_params[result]
    : LPAREN param_decl_list[params] RPAREN
      {
        // Register input parameters in scope (read-only)
        for (const auto& param : $params) {
          if (is_redeclaration(param->name, true)) {
            error(@params, "State input parameter '" + param->name + "' conflicts with autotuner-level declaration");
            YYABORT;
          }
          state_input_params.insert(param->name);
        }
        $result = std::move($params);
      }
    | LPAREN RPAREN
      { 
        $result = std::vector<std::unique_ptr<ParamDecl>>(); 
      }
    | %empty
      { 
        $result = std::vector<std::unique_ptr<ParamDecl>>(); 
      }
    ;

// ============================================================================
// TYPES
// ============================================================================

type_spec[result]
    : INT_KW           
      { $result = std::make_unique<TypeDescriptor>(ParamType::Int); }
    | FLOAT_KW         
      { $result = std::make_unique<TypeDescriptor>(ParamType::Float); }
    | BOOL_KW          
      { $result = std::make_unique<TypeDescriptor>(ParamType::Bool); }
    | STRING_KW        
      { $result = std::make_unique<TypeDescriptor>(ParamType::String); }
    | ERROR_KW         
      { $result = std::make_unique<TypeDescriptor>(ParamType::Error, "Error"); }
    | IDENTIFIER[name]
      {
        // Allow user-defined struct type names declared earlier in this file.
        if (struct_known_types.count($name) == 0) {
          error(@name, "Unknown type '" + $name + "' — "
                "did you forget to declare a struct with this name before use?");
          YYABORT;
        }
        $result = std::make_unique<TypeDescriptor>(
            TypeDescriptor::make_struct($name));
      }

// ============================================================================
// AUTOTUNER COMPONENTS
// ============================================================================

requires_clause[result]
    : USES identifier_list[required_deps] SEMICOLON
      { 
        $result = std::move($required_deps); 
      }
    | %empty
      { 
        $result = std::vector<std::string>(); 
      }
    ;

identifier_list[result]
    : IDENTIFIER[first_id]
      {
        $result = std::vector<std::string>();
        $result.push_back(std::move($first_id));
      }
    | identifier_list[existing_ids] COMMA IDENTIFIER[next_id]
      {
        $result = std::move($existing_ids);
        $result.push_back(std::move($next_id));
      }
    | %empty
      { 
        $result = std::vector<std::string>(); 
      }
    ;

autotuner_var_decls[result]
    : %empty
      {
        $result = std::vector<std::unique_ptr<Stmt>>();
      }
    | autotuner_var_decls[existing_vars] stmt[next_var]
      {
        $result = std::move($existing_vars);
        $result.push_back(std::move($next_var));
      }
    ;

entry_state[result]
    : START ARROW IDENTIFIER[state_name]
      { 
        $result = std::move($state_name); 
      }
    ;

entry_params[result]
    : LPAREN expr_list[params] RPAREN
      { 
        $result = std::move($params); 
      }
    | %empty
      { 
        $result = std::vector<std::unique_ptr<Expr>>(); 
      }
    ;

// ============================================================================
// STATES
// ============================================================================

state_list[result]
    : state_decl[first_state]
      {
        $result = std::vector<StateDecl>();
        $result.push_back(std::move(*$first_state));
      }
    | state_list[existing_states] state_decl[next_state]
      {
        $result = std::move($existing_states);
        $result.push_back(std::move(*$next_state));
      }
    ;

state_decl[result]
    : STATE IDENTIFIER[name] 
      {
        // Clear state-local scope when entering new state
        clear_state_scope();
      }
      state_input_params[input_param] 
      LBRACE 
        stmt_list[body] 
      RBRACE
      {
        $result = std::make_unique<StateDecl>(
          std::move($name), 
          std::move($input_param), 
          std::move($body)
        );
      }
    ;

// ============================================================================
// STATEMENTS
// ============================================================================

stmt_list[result]
    : %empty
      { $result = std::vector<std::unique_ptr<Stmt>>(); }
    | stmt_list[existing_stmts] stmt[next_stmt]
      {
        $result = std::move($existing_stmts);
        $result.push_back(std::move($next_stmt));
      }
    ;

stmt[result]
    : var_decl_stmt[var_decl]
      { 
        $result = std::move($var_decl); 
        set_stmt_location($result.get(), @var_decl);
      }
    | IDENTIFIER[name] ASSIGN expr[val] SEMICOLON
      {
        if (!is_variable_declared($name)) {
          error(@name, "Assignment to undeclared variable: " + $name);
          YYABORT;
        }
        if (autotuner_input_params.count($name) > 0 ||
            state_input_params.count($name) > 0) {
          error(@name, "Cannot assign to read-only input parameter: " + $name);
          YYABORT;
        }
        std::vector<AssignTarget> targets;
        targets.emplace_back($name);
        $result = std::make_unique<AssignStmt>(std::move(targets), std::move($val));
        set_stmt_location($result.get(), @name);
      }
    // Tuple / mixed assignment: a, q.field = func();  or  a, b = func();
    | assign_target_list[tgts] ASSIGN expr[val] SEMICOLON
      {
        $result = std::make_unique<AssignStmt>(std::move($tgts), std::move($val));
        set_stmt_location($result.get(), @tgts);
      }
    // Standalone dot-field assignment: q.field = expr;
    | expr[object] DOT IDENTIFIER[field] ASSIGN expr[val] SEMICOLON
      {
        $result = std::make_unique<StructFieldAssignStmt>(
            std::move($object), std::move($field), std::move($val));
        set_stmt_location($result.get(), @object);
      }
    | expr[side_effect_expr] SEMICOLON
      {
        $result = std::make_unique<ExprStmt>(std::move($side_effect_expr));
        set_stmt_location($result.get(), @side_effect_expr);
      }
    | IF LPAREN expr[condition] RPAREN LBRACE stmt_list[then_body] RBRACE elif_chain[else_body]
      {
        $result = std::make_unique<IfStmt>(
          std::move($condition),
          std::move($then_body),
          std::move($else_body)
        );
        set_stmt_location($result.get(), @IF);
      }
    | ARROW IDENTIFIER[target_state] SEMICOLON
      {
        $result = std::make_unique<TransitionStmt>(
          std::move($target_state)
        );
        set_stmt_location($result.get(), @ARROW);
      }
    | ARROW IDENTIFIER[target_state] LPAREN expr_list[params] RPAREN SEMICOLON
      {
        $result = std::make_unique<TransitionStmt>(
          std::move($target_state), 
          std::move($params)
        );
        set_stmt_location($result.get(), @ARROW);
      }
    | TERMINAL SEMICOLON
      {
        $result = std::make_unique<TerminalStmt>();
        set_stmt_location($result.get(), @TERMINAL);
      }
    ;

// A comma-separated list of assignment targets (>=2 items, for tuple assigns).
// Allows mixing plain variables and struct-field targets.
// Example: a, q.field, b
assign_target_list[result]
    : assign_target[first] COMMA assign_target[second]
      {
        $result = std::vector<AssignTarget>();
        $result.push_back(std::move(*$first));
        $result.push_back(std::move(*$second));
      }
    | assign_target_list[prev] COMMA assign_target[next]
      {
        $result = std::move($prev);
        $result.push_back(std::move(*$next));
      }
    ;

assign_target[result]
    : IDENTIFIER[name]
      {
        if (!is_variable_declared($name)) {
          error(@name, "Assignment to undeclared variable: " + $name);
          YYABORT;
        }
        $result = std::make_unique<AssignTarget>($name);
      }
    | expr[object] DOT IDENTIFIER[field]
      {
        $result = std::make_unique<AssignTarget>(std::move($object), std::move($field));
      }
    ;

elif_chain[result]
    : ELIF LPAREN expr[condition] RPAREN LBRACE stmt_list[then_body] RBRACE elif_chain[next_else]
      {
        std::vector<std::unique_ptr<Stmt>> else_vec;
        else_vec.push_back(std::make_unique<IfStmt>(
          std::move($condition),
          std::move($then_body),
          std::move($next_else)
        ));
        $result = std::move(else_vec);
      }
    | ELSE LBRACE stmt_list[else_body] RBRACE
      { $result = std::move($else_body); }
    | /* empty */
       %empty { $result = std::vector<std::unique_ptr<Stmt>>(); }
    ;

var_decl_stmt[result]
    : type_spec[type] IDENTIFIER[name] SEMICOLON
      {
        // Determine if we're in a state or at autotuner level
        bool in_state = !state_local_scope.empty() || !state_input_params.empty() || 
                       (current_errors.empty()); // Heuristic: if no errors, assume we're parsing normally
        
        // Check for redeclaration
        if (is_redeclaration($name, in_state)) {
          error(@name, "Redeclaration of variable '" + $name + "' (conflicts with existing declaration)");
          YYABORT;
        }
        
        // Add to appropriate scope
        if (in_state) {
          state_local_scope.insert($name);
        } else {
          autotuner_scope.insert($name);
        }
        
        $result = std::make_unique<VarDeclStmt>(
          std::move(*$type), 
          std::move($name), 
          std::nullopt
        );
      }
    | type_spec[type] IDENTIFIER[name] ASSIGN expr[initializer] SEMICOLON
      {
        // Determine if we're in a state or at autotuner level
        bool in_state = !state_local_scope.empty() || !state_input_params.empty();
        
        // Check for redeclaration
        if (is_redeclaration($name, in_state)) {
          error(@name, "Redeclaration of variable '" + $name + "' (conflicts with existing declaration)");
          YYABORT;
        }
        
        // Add to appropriate scope
        if (in_state) {
          state_local_scope.insert($name);
        } else {
          autotuner_scope.insert($name);
        }
        
        $result = std::make_unique<VarDeclStmt>(
          std::move(*$type), 
          std::move($name), 
          std::make_optional(std::move($initializer))
        );
      }
    ;

// ============================================================================
// EXPRESSIONS
// ============================================================================

expr[result]
    : primary_expr[prim]
      { 
        $result = std::move($prim); 
      }
    | postfix_expr[postfix]
      { 
        $result = std::move($postfix); 
      }
    | expr[left] PLUS expr[right]
      { 
        $result = std::make_unique<BinaryExpr>("+", std::move($left), std::move($right)); 
      }
    | expr[left] MINUS expr[right]
      { 
        $result = std::make_unique<BinaryExpr>("-", std::move($left), std::move($right)); 
      }
    | expr[left] MUL expr[right]
      { 
        $result = std::make_unique<BinaryExpr>("*", std::move($left), std::move($right)); 
      }
    | expr[left] DIV expr[right]
      { 
        $result = std::make_unique<BinaryExpr>("/", std::move($left), std::move($right)); 
      }
    | expr[left] EQ expr[right]
      { 
        $result = std::make_unique<BinaryExpr>("==", std::move($left), std::move($right)); 
      }
    | expr[left] NE expr[right]
      { 
        $result = std::make_unique<BinaryExpr>("!=", std::move($left), std::move($right)); 
      }
    | expr[left] LL expr[right]
      { 
        $result = std::make_unique<BinaryExpr>("<", std::move($left), std::move($right)); 
      }
    | expr[left] GG expr[right]
      { 
        $result = std::make_unique<BinaryExpr>(">", std::move($left), std::move($right)); 
      }
    | expr[left] LE expr[right]
      { 
        $result = std::make_unique<BinaryExpr>("<=", std::move($left), std::move($right)); 
      }
    | expr[left] GE expr[right]
      { 
        $result = std::make_unique<BinaryExpr>(">=", std::move($left), std::move($right)); 
      }
    | expr[left] AND expr[right]
      { 
        $result = std::make_unique<BinaryExpr>("&&", std::move($left), std::move($right)); 
      }
    | expr[left] OR expr[right]
      { 
        $result = std::make_unique<BinaryExpr>("||", std::move($left), std::move($right)); 
      }
    | NOT expr[operand]
      { 
        $result = std::make_unique<UnaryExpr>("!", std::move($operand)); 
      }
    | MINUS expr[operand] %prec UMINUS
      { 
        $result = std::make_unique<UnaryExpr>("-", std::move($operand)); 
      }
    ;

postfix_expr[result]
    : expr[object] DOT IDENTIFIER[member_name]
      { 
        $result = std::make_unique<MemberExpr>(
          std::move($object), 
          std::move($member_name)
        ); 
      }
    | expr[object] DOT IDENTIFIER[method_name] LPAREN expr_list[args] RPAREN
      { 
        $result = std::make_unique<MethodCallExpr>(
          std::move($object), 
          std::move($method_name), 
          std::move($args)
        ); 
      }
    | expr[array] LBRACKET expr[index] RBRACKET
      { 
        $result = std::make_unique<IndexExpr>(
          std::move($array), 
          std::move($index)
        ); 
      }
    | IDENTIFIER[function_name] LPAREN RPAREN
      {
        $result = std::make_unique<CallExpr>(
          std::move($function_name),
          std::vector<CallArg>() // empty argument list
        );
      }
    | IDENTIFIER[function_name] LPAREN call_arg_list[call_args] RPAREN
      {
        $result = std::make_unique<CallExpr>(
          std::move($function_name),
          std::move($call_args)
        );
      }
    ;

primary_expr[result]
    : INTEGER[int_value]
      { 
        $result = std::make_unique<LiteralExpr>(std::stoll($int_value)); 
      }
    | DOUBLE[float_value]
      { 
        $result = std::make_unique<LiteralExpr>(std::stod($float_value)); 
      }
    | STRING[string_value]
      { 
        $result = std::make_unique<LiteralExpr>(std::move($string_value)); 
      }
    | TRUE
      { 
        $result = std::make_unique<LiteralExpr>(true); 
      }
    | FALSE
      { 
        $result = std::make_unique<LiteralExpr>(false); 
      }
    | NIL
      { 
        $result = std::make_unique<NilLiteralExpr>(); 
      }
    | IDENTIFIER[var_name]
      {
        // Validate that variable is declared (only check for variables, not function calls)
        // Note: We can't easily distinguish here, so we only warn, not error
        // The semantic analyzer will do a full check later
        
        $result = std::make_unique<VarExpr>(std::move($var_name)); 
      }
    | LPAREN expr[inner] RPAREN
      { 
        $result = std::move($inner); 
      }
    ;

expr_list[result]
    : expr[first_expr]
      {
        $result = std::vector<std::unique_ptr<Expr>>();
        $result.push_back(std::move($first_expr));
      }
    | expr_list[existing_exprs] COMMA expr[next_expr]
      {
        $result = std::move($existing_exprs);
        $result.push_back(std::move($next_expr));
      }
    ;

call_arg_list[result]
    : call_arg[first_arg]
      {
        $result = std::vector<CallArg>();
        $result.push_back(std::move(*$first_arg));
      }
    | call_arg_list[existing_args] COMMA call_arg[next_arg]
      {
        $result = std::move($existing_args);
        $result.push_back(std::move(*$next_arg));
      }
    ;

call_arg[result]
    : expr[arg_expr]
      {
        $result = std::make_unique<CallArg>(std::move($arg_expr));
      }
    | IDENTIFIER[param_name] ASSIGN expr[param_value]
      {
        $result = std::make_unique<CallArg>(std::move($param_name), std::move($param_value));
      }
    ;

%%

void falcon::atc::Parser::error(const location_type& loc, const std::string& msg) {
  ParseError err{
    loc.begin.line,
    loc.begin.column,
    loc.end.line,
    loc.end.column,
    msg
  };
  
  current_errors.push_back(err);
}
