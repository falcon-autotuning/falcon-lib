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

  // ── Scope tracking for variable declarations ─────────────────────────────
  std::set<std::string> autotuner_scope;
  std::set<std::string> autotuner_input_params;
  std::set<std::string> autotuner_output_params;
  std::set<std::string> state_local_scope;
  std::set<std::string> state_input_params;
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

  // ── Struct context tracking ───────────────────────────────────────────────
  // struct_known_types is DEFINED in ParseError.cpp and DECLARED extern in
  // ParseError.hpp (which is included via %code requires above).
  // Do NOT redefine it here — that would shadow the cross-TU definition and
  // break the Compiler.cpp → struct_known_types linkage.

  // module_known_types is parser-session-only (not needed cross-TU).
  std::set<std::string> module_known_types;

  // struct_field_scope tracks field names inside the struct being parsed.
  std::set<std::string> struct_field_scope;

  // True while parsing a routine body that belongs to a struct.
  bool in_struct_routine = false;

  void enter_struct_routine() {
    in_struct_routine = true;
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
    if (in_struct_routine && struct_field_scope.count(name) > 0) return true;
    return autotuner_scope.count(name) > 0 ||
           autotuner_input_params.count(name) > 0 ||
           autotuner_output_params.count(name) > 0 ||
           state_local_scope.count(name) > 0 ||
           state_input_params.count(name) > 0;
  }

  bool is_redeclaration(const std::string& name, bool in_state) {
    if (in_state) {
      return autotuner_scope.count(name) > 0 ||
             autotuner_input_params.count(name) > 0 ||
             autotuner_output_params.count(name) > 0 ||
             state_input_params.count(name) > 0 ||
             state_local_scope.count(name) > 0;
    } else {
      return autotuner_scope.count(name) > 0 ||
             autotuner_input_params.count(name) > 0 ||
             autotuner_output_params.count(name) > 0;
    }
  }

  void set_stmt_location(falcon::atc::Stmt* stmt,
                         const falcon::atc::Parser::location_type& loc) {
    if (stmt) {
      stmt->filename = falcon::atc::current_filename;
      stmt->line     = loc.begin.line;
      stmt->column   = loc.begin.column;
    }
  }
}

// ============================================================================
// TOKEN DECLARATIONS
// ============================================================================

%token AUTOTUNER ROUTINE STATE IMPORT FFIMPORT START USES TERMINAL
%token IF ELIF ELSE STRUCT THIS
%token <std::string> IDENTIFIER STRING INTEGER DOUBLE
%token INT_KW FLOAT_KW BOOL_KW STRING_KW ERROR_KW
%token TRUE FALSE NIL
%token ARROW ASSIGN SEMICOLON COMMA DOT
%token LBRACE RBRACE LPAREN RPAREN LBRACKET RBRACKET
%token COLONCOLON
%token PLUS MINUS MUL DIV
%token EQ NE LL GG LE GE AND OR NOT

%type <std::unique_ptr<AutotunerDecl>> autotuner_decl
%type <std::vector<std::unique_ptr<ParamDecl>>> input_params output_params param_list state_params
%type <std::unique_ptr<ParamDecl>> param_decl
%type <std::unique_ptr<TypeDescriptor>> type_spec
%type <std::unique_ptr<FFImportDecl>> ffimport_decl
%type <std::vector<std::string>> requires_clause identifier_list import_list import_stmt import_string_list ffimport_string_list
%type <std::vector<std::unique_ptr<Stmt>>> autotuner_var_decls routine_body routine_body_stmts 
%type <std::unique_ptr<VarDeclStmt>> var_decl_stmt struct_field_decl
%type <std::string> entry_state qualified_name
%type <std::vector<std::unique_ptr<Expr>>> entry_params
%type <std::vector<StateDecl>> state_list
%type <std::unique_ptr<StateDecl>> state_decl
%type <std::vector<std::unique_ptr<Stmt>>> stmt_list elif_chain
%type <std::unique_ptr<Stmt>> stmt struct_routine_stmt
%type <std::unique_ptr<Expr>> expr primary_expr postfix_expr
%type <std::vector<std::unique_ptr<Expr>>> expr_list 
%type <std::vector<CallArg>> call_arg_list 
%type <std::unique_ptr<CallArg>> call_arg
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
    std::unique_ptr<AutotunerDecl>,
    std::unique_ptr<FFImportDecl>
>> > program_items

%type <std::variant<
    std::unique_ptr<StructDecl>,
    std::unique_ptr<RoutineDecl>,
    std::unique_ptr<AutotunerDecl>,
    std::unique_ptr<FFImportDecl>
>> program_item

%left OR
%left AND
%left EQ NE 
%left LL GG LE GE
%left PLUS MINUS
%left MUL DIV
%right NOT UMINUS
%left DOT LBRACKET LPAREN COLONCOLON

%start program

%%

// ============================================================================
// PROGRAM STRUCTURE
// ============================================================================

program
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
          } else if (std::holds_alternative<std::unique_ptr<FFImportDecl>>(item)) {
            prog->ff_imports.push_back(std::move(*std::get<std::unique_ptr<FFImportDecl>>(item)));
          }
        }
        prog->build_indexes();
        program_root = std::move(prog);
        // NOTE: do NOT move program_root into $result — parse_file() reads
        // program_root directly via the extern global.  Moving it into $result
        // (which bison discards for the start symbol) would leave program_root null.
      }
    ;

program_items[result]
    : %empty
      { $result = std::vector<std::variant<std::unique_ptr<StructDecl>,std::unique_ptr<RoutineDecl>,std::unique_ptr<AutotunerDecl>,std::unique_ptr<FFImportDecl>>>(); }
    | program_items[prev] program_item[next]
      {
        $result = std::move($prev);
        $result.push_back(std::move($next));
      }
    ;

program_item[result]
    : struct_decl[s]      { $result = std::move($s); }
    | routine_decl[r]     { $result = std::move($r); }
    | autotuner_decl[a]   { $result = std::move($a); }
    | ffimport_decl[f]    { $result = std::move($f); }
    ;

// ============================================================================
// QUALIFIED NAME  (Module::symbol  OR  plain  symbol)
// ============================================================================
// A qualified_name produces a single string "Module::symbol" that the runtime
// can split on "::" to resolve imports.  Plain IDENTIFIER is also a valid
// qualified_name (no module prefix).

qualified_name[result]
    : IDENTIFIER[ns] COLONCOLON IDENTIFIER[sym]
      { $result = $ns + "::" + $sym; }
    | IDENTIFIER[name]
      { $result = $name; }
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
        struct_field_scope.clear();
        struct_known_types.insert($name);
      }
      struct_field_list[fields] struct_routine_list[routines] RBRACE
      {
        struct_field_scope.clear();
        $result = std::make_unique<StructDecl>(
            std::move($name),
            std::move($fields),
            std::move($routines));
      }
    ;

// ---------------------------------------------------------------------------
// Struct field declarations
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

// ---------------------------------------------------------------------------
// Body of a struct routine
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
    : var_decl_stmt[vd]
      {
        $result = std::move($vd);
        set_stmt_location($result.get(), @vd);
      }
    | IDENTIFIER[name] ASSIGN expr[val] SEMICOLON
      {
        if (struct_field_scope.count($name) > 0) {
          $result = std::make_unique<StructFieldAssignStmt>(
              std::make_unique<VarExpr>("this"),
              std::move($name),
              std::move($val));
        } else if (is_variable_declared($name)) {
          if (autotuner_input_params.count($name) > 0 ||
              state_input_params.count($name) > 0) {
            error(@name, "Cannot assign to read-only parameter: " + $name);
          }
          std::vector<AssignTarget> targets;
          targets.push_back(AssignTarget($name));
          $result = std::make_unique<AssignStmt>(
              std::move(targets),
              std::move($val));
        } else {
          error(@name, "Undefined variable: " + $name);
          YYABORT;
        }
        set_stmt_location($result.get(), @name);
      }
    | IDENTIFIER[object] DOT IDENTIFIER[field] ASSIGN expr[val] SEMICOLON
      {
        $result = std::make_unique<StructFieldAssignStmt>(
            std::make_unique<VarExpr>(std::move($object)),
            std::move($field),
            std::move($val));
        set_stmt_location($result.get(), @object);
      }
    | THIS DOT IDENTIFIER[field] ASSIGN expr[val] SEMICOLON
      {
        $result = std::make_unique<StructFieldAssignStmt>(
            std::make_unique<VarExpr>("this"),
            std::move($field),
            std::move($val));
        set_stmt_location($result.get(), @THIS);
      }
    | IF LPAREN expr[cond] RPAREN LBRACE routine_body[then_b] RBRACE elif_chain[else_b]
      {
        $result = std::make_unique<IfStmt>(
            std::move($cond),
            std::move($then_b),
            std::move($else_b));
        set_stmt_location($result.get(), @IF);
      }
    | expr[e] SEMICOLON
      {
        $result = std::make_unique<ExprStmt>(std::move($e));
        set_stmt_location($result.get(), @e);
      }
    ;

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

ffimport_decl[result]
    : FFIMPORT STRING[wrapper]
      LPAREN ffimport_string_list[imports] RPAREN
      LPAREN ffimport_string_list[libs] RPAREN
      {
        $result = std::make_unique<FFImportDecl>(
          std::move($wrapper),
          std::move($imports),
          std::move($libs)
        );
      }
    ;

ffimport_string_list[result]
    : STRING[first]
      { $result = std::vector<std::string>{std::move($first)}; }
    | ffimport_string_list[existing] STRING[next]
      {
        $result = std::move($existing);
        $result.push_back(std::move($next));
      }
    | %empty
      { $result = std::vector<std::string>(); }
    ;

import_list[result]
    : %empty
       { $result = std::vector<std::string>(); }
    | import_list[existing] import_stmt[next] 
      { 
        $result = std::move($existing);
        $result.insert($result.end(),
                       std::make_move_iterator($next.begin()),
                       std::make_move_iterator($next.end()));
      }
    ;

// ============================================================================
// AUTOTUNER DECLARATION
// ============================================================================

autotuner_decl[result]
    : AUTOTUNER IDENTIFIER[name]
      {
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
    : LPAREN param_list[params] RPAREN
      {
        for (const auto& p : $params) {
          autotuner_input_params.insert(p->name);
        }
        $result = std::move($params);
      }
    | LPAREN RPAREN
      { $result = std::vector<std::unique_ptr<ParamDecl>>(); }
    | %empty
      { $result = std::vector<std::unique_ptr<ParamDecl>>(); }
    ;

output_params[result]
    : LPAREN param_list[params] RPAREN
      {
        for (const auto& p : $params) {
          autotuner_output_params.insert(p->name);
        }
        $result = std::move($params);
      }
    | LPAREN RPAREN
      { $result = std::vector<std::unique_ptr<ParamDecl>>(); }
    ;

param_list[result]
    : param_decl[first]
      {
        $result = std::vector<std::unique_ptr<ParamDecl>>();
        $result.push_back(std::move($first));
      }
    | param_list[existing] COMMA param_decl[next]
      {
        $result = std::move($existing);
        $result.push_back(std::move($next));
      }
    ;

param_decl[result]
    : type_spec[type] IDENTIFIER[name]
      {
        $result = std::make_unique<ParamDecl>(std::move(*$type), std::move($name));
      }
    | type_spec[type] IDENTIFIER[name] ASSIGN expr[default_val]
      {
        $result = std::make_unique<ParamDecl>(
            std::move(*$type), std::move($name),
            std::make_optional(std::move($default_val)));
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
        // Allow plain struct names OR module-registered struct names.
        // module_known_types holds "Module::TypeName"-style keys but we also
        // allow the bare type name if it was registered from an import.
        if (struct_known_types.count($name) == 0 &&
            module_known_types.count($name) == 0) {
          error(@name, "Unknown type '" + $name + "' — "
                "did you forget to declare a struct with this name before use?");
          YYABORT;
        }
        $result = std::make_unique<TypeDescriptor>(
            TypeDescriptor::make_struct($name));
      }
    ;

// ============================================================================
// AUTOTUNER COMPONENTS
// ============================================================================

requires_clause[result]
    : USES identifier_list[required_deps] SEMICOLON
      { $result = std::move($required_deps); }
    | %empty
      { $result = std::vector<std::string>(); }
    ;

// identifier_list now supports both plain names and Module::symbol qualifiers
identifier_list[result]
    : qualified_name[first_id]
      {
        $result = std::vector<std::string>();
        $result.push_back(std::move($first_id));
      }
    | identifier_list[existing_ids] COMMA qualified_name[next_id]
      {
        $result = std::move($existing_ids);
        $result.push_back(std::move($next_id));
      }
    | %empty
      { $result = std::vector<std::string>(); }
    ;

autotuner_var_decls[result]
    : %empty
      { $result = std::vector<std::unique_ptr<Stmt>>(); }
    | autotuner_var_decls[existing_vars] stmt[next_var]
      {
        $result = std::move($existing_vars);
        $result.push_back(std::move($next_var));
      }
    ;

entry_state[result]
    : START ARROW IDENTIFIER[state_name]
      { $result = std::move($state_name); }
    ;

entry_params[result]
    : LPAREN expr_list[params] RPAREN
      { $result = std::move($params); }
    | %empty
      { $result = std::vector<std::unique_ptr<Expr>>(); }
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
        clear_state_scope();
      }
      state_params[params] LBRACE stmt_list[stmts] RBRACE
      {
        $result = std::make_unique<StateDecl>(
          std::move($name),
          std::move($params),
          std::move($stmts)
        );
      }
    ;

state_params[result]
    : LPAREN param_list[params] RPAREN
      {
        for (const auto& p : $params) {
          state_input_params.insert(p->name);
        }
        $result = std::move($params);
      }
    | LPAREN RPAREN
      { $result = std::vector<std::unique_ptr<ParamDecl>>(); }
    | %empty
      { $result = std::vector<std::unique_ptr<ParamDecl>>(); }
    ;

// ============================================================================
// STATEMENTS
// ============================================================================

stmt_list[result]
    : %empty
      { $result = std::vector<std::unique_ptr<Stmt>>(); }
    | stmt_list[existing] stmt[next]
      {
        $result = std::move($existing);
        $result.push_back(std::move($next));
      }
    ;

stmt[result]
    : var_decl_stmt[vd]
      {
        $result = std::move($vd);
        set_stmt_location($result.get(), @vd);
      }
    | assign_target_list[targets] ASSIGN expr[val] SEMICOLON
      {
        $result = std::make_unique<AssignStmt>(
            std::move($targets),
            std::move($val));
        set_stmt_location($result.get(), @targets);
      }
    | IDENTIFIER[object] DOT IDENTIFIER[field] ASSIGN expr[val] SEMICOLON
      {
        $result = std::make_unique<StructFieldAssignStmt>(
            std::make_unique<VarExpr>(std::move($object)),
            std::move($field),
            std::move($val));
        set_stmt_location($result.get(), @object);
      }
    | THIS DOT IDENTIFIER[field] ASSIGN expr[val] SEMICOLON
      {
        $result = std::make_unique<StructFieldAssignStmt>(
            std::make_unique<VarExpr>("this"),
            std::move($field),
            std::move($val));
        set_stmt_location($result.get(), @THIS);
      }
    | ARROW IDENTIFIER[target] SEMICOLON
      {
        $result = std::make_unique<TransitionStmt>(
            std::move($target),
            std::vector<std::unique_ptr<Expr>>());
        set_stmt_location($result.get(), @ARROW);
      }
    | ARROW IDENTIFIER[target] LPAREN expr_list[args] RPAREN SEMICOLON
      {
        $result = std::make_unique<TransitionStmt>(
            std::move($target),
            std::move($args));
        set_stmt_location($result.get(), @ARROW);
      }
    | TERMINAL SEMICOLON
      {
        $result = std::make_unique<TerminalStmt>();
        set_stmt_location($result.get(), @TERMINAL);
      }
    | IF LPAREN expr[cond] RPAREN LBRACE stmt_list[then_b] RBRACE elif_chain[else_b]
      {
        $result = std::make_unique<IfStmt>(
            std::move($cond),
            std::move($then_b),
            std::move($else_b));
        set_stmt_location($result.get(), @IF);
      }
    | expr[e] SEMICOLON
      {
        $result = std::make_unique<ExprStmt>(std::move($e));
        set_stmt_location($result.get(), @e);
      }
    ;

elif_chain[result]
    : %empty
      { $result = std::vector<std::unique_ptr<Stmt>>(); }
    | ELSE LBRACE stmt_list[else_b] RBRACE
      { $result = std::move($else_b); }
    | ELIF LPAREN expr[cond] RPAREN LBRACE stmt_list[then_b] RBRACE elif_chain[rest]
      {
        $result = std::vector<std::unique_ptr<Stmt>>();
        $result.push_back(std::make_unique<IfStmt>(
            std::move($cond),
            std::move($then_b),
            std::move($rest)));
      }
    ;

assign_target_list[result]
    : assign_target[first]
      {
        $result = std::vector<AssignTarget>();
        $result.push_back(std::move(*$first));
      }
    | assign_target_list[existing] COMMA assign_target[next]
      {
        $result = std::move($existing);
        $result.push_back(std::move(*$next));
      }
    ;

assign_target[result]
    : IDENTIFIER[name]
      {
        if (!is_variable_declared($name)) {
          error(@name, "Undefined variable: " + $name);
          YYABORT;
        }
        if (autotuner_input_params.count($name) > 0 ||
            state_input_params.count($name) > 0) {
          error(@name, "Cannot assign to read-only parameter: " + $name);
        }
        $result = std::make_unique<AssignTarget>($name);
      }
    ;

var_decl_stmt[result]
    : type_spec[type] IDENTIFIER[name] SEMICOLON
      {
        if (is_redeclaration($name, !state_local_scope.empty() || !state_input_params.empty())) {
          error(@name, "Redeclaration of variable: " + $name);
        }
        if (!in_struct_routine) {
          if (!state_local_scope.empty() || !state_input_params.empty()) {
            state_local_scope.insert($name);
          } else {
            autotuner_scope.insert($name);
          }
        }
        $result = std::make_unique<VarDeclStmt>(std::move(*$type), std::move($name), std::nullopt);
        set_stmt_location($result.get(), @name);
      }
    | type_spec[type] IDENTIFIER[name] ASSIGN expr[init] SEMICOLON
      {
        if (is_redeclaration($name, !state_local_scope.empty() || !state_input_params.empty())) {
          error(@name, "Redeclaration of variable: " + $name);
        }
        if (!in_struct_routine) {
          if (!state_local_scope.empty() || !state_input_params.empty()) {
            state_local_scope.insert($name);
          } else {
            autotuner_scope.insert($name);
          }
        }
        $result = std::make_unique<VarDeclStmt>(
            std::move(*$type), std::move($name),
            std::make_optional(std::move($init)));
        set_stmt_location($result.get(), @name);
      }
    ;

// ============================================================================
// ROUTINE DECLARATION
// ============================================================================

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

// ============================================================================
// EXPRESSIONS
// ============================================================================

expr[result]
    : primary_expr[e]     { $result = std::move($e); }
    | postfix_expr[e]     { $result = std::move($e); }
    | expr[l] PLUS  expr[r] { $result = std::make_unique<BinaryExpr>("+",  std::move($l), std::move($r)); }
    | expr[l] MINUS expr[r] { $result = std::make_unique<BinaryExpr>("-",  std::move($l), std::move($r)); }
    | expr[l] MUL   expr[r] { $result = std::make_unique<BinaryExpr>("*",  std::move($l), std::move($r)); }
    | expr[l] DIV   expr[r] { $result = std::make_unique<BinaryExpr>("/",  std::move($l), std::move($r)); }
    | expr[l] EQ    expr[r] { $result = std::make_unique<BinaryExpr>("==", std::move($l), std::move($r)); }
    | expr[l] NE    expr[r] { $result = std::make_unique<BinaryExpr>("!=", std::move($l), std::move($r)); }
    | expr[l] LL    expr[r] { $result = std::make_unique<BinaryExpr>("<",  std::move($l), std::move($r)); }
    | expr[l] GG    expr[r] { $result = std::make_unique<BinaryExpr>(">",  std::move($l), std::move($r)); }
    | expr[l] LE    expr[r] { $result = std::make_unique<BinaryExpr>("<=", std::move($l), std::move($r)); }
    | expr[l] GE    expr[r] { $result = std::make_unique<BinaryExpr>(">=", std::move($l), std::move($r)); }
    | expr[l] AND   expr[r] { $result = std::make_unique<BinaryExpr>("&&", std::move($l), std::move($r)); }
    | expr[l] OR    expr[r] { $result = std::make_unique<BinaryExpr>("||", std::move($l), std::move($r)); }
    | NOT  expr[e]  { $result = std::make_unique<UnaryExpr>("!",  std::move($e)); }
    | MINUS expr[e] %prec UMINUS { $result = std::make_unique<UnaryExpr>("-", std::move($e)); }
    | LPAREN expr[e] RPAREN { $result = std::move($e); }
    ;

primary_expr[result]
    : INTEGER[v]  { $result = std::make_unique<LiteralExpr>(static_cast<int64_t>(std::stoll($v))); }
    | DOUBLE[v]   { $result = std::make_unique<LiteralExpr>(std::stod($v)); }
    | STRING[v]   { $result = std::make_unique<LiteralExpr>($v); }
    | TRUE        { $result = std::make_unique<LiteralExpr>(true); }
    | FALSE       { $result = std::make_unique<LiteralExpr>(false); }
    | NIL         { $result = std::make_unique<NilLiteralExpr>(); }
    | IDENTIFIER[name] { $result = std::make_unique<VarExpr>($name); }
    // Module-qualified variable/type reference: Module::symbol
    // Produces a VarExpr with the fully-qualified name so the runtime
    // can resolve it through the import namespace map.
    | IDENTIFIER[ns] COLONCOLON IDENTIFIER[sym]
      { $result = std::make_unique<VarExpr>($ns + "::" + $sym); }
    ;

postfix_expr[result]
    // Member access: expr.field
    : postfix_expr[obj] DOT IDENTIFIER[field]
      { $result = std::make_unique<MemberExpr>(std::move($obj), std::move($field)); }
    // Method call: expr.method(args)
    | postfix_expr[obj] DOT IDENTIFIER[method] LPAREN call_arg_list[args] RPAREN
      {
        std::vector<std::unique_ptr<Expr>> arg_exprs;
        arg_exprs.reserve($args.size());
        for (auto& a : $args) { arg_exprs.push_back(std::move(a.value)); }
        $result = std::make_unique<MethodCallExpr>(std::move($obj), std::move($method), std::move(arg_exprs));
      }
    // Index: expr[idx]
    | postfix_expr[obj] LBRACKET expr[idx] RBRACKET
      { $result = std::make_unique<IndexExpr>(std::move($obj), std::move($idx)); }
    // Plain function call: name(args)
    | IDENTIFIER[name] LPAREN call_arg_list[args] RPAREN
      {
        $result = std::make_unique<CallExpr>(std::move($name), std::move($args));
      }
    // Module-qualified function call: Module::symbol(args)
    | IDENTIFIER[ns] COLONCOLON IDENTIFIER[sym] LPAREN call_arg_list[args] RPAREN
      {
        // Encode as a CallExpr with the qualified name; the runtime resolves it.
        $result = std::make_unique<CallExpr>($ns + "::" + $sym, std::move($args));
      }
    | primary_expr[e] { $result = std::move($e); }
    ;

expr_list[result]
    : expr[first]
      {
        $result = std::vector<std::unique_ptr<Expr>>();
        $result.push_back(std::move($first));
      }
    | expr_list[existing] COMMA expr[next]
      {
        $result = std::move($existing);
        $result.push_back(std::move($next));
      }
    ;

call_arg_list[result]
    : %empty
      { $result = std::vector<CallArg>(); }
    | call_arg[first]
      {
        $result = std::vector<CallArg>();
        $result.push_back(std::move(*$first));
      }
    | call_arg_list[existing] COMMA call_arg[next]
      {
        $result = std::move($existing);
        $result.push_back(std::move(*$next));
      }
    ;

call_arg[result]
    : IDENTIFIER[name] ASSIGN expr[val]
      { $result = std::make_unique<CallArg>(std::move($name), std::move($val)); }
    | expr[val]
      { $result = std::make_unique<CallArg>(std::move($val)); }
    ;

%%

namespace falcon::atc {
  void Parser::error(const location_type& loc, const std::string& msg) {
    current_errors.push_back(ParseError{
        loc.begin.line,
        loc.begin.column,
        loc.end.line,
        loc.end.column,
        msg
    });
  }
}
