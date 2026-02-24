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
  
  void clear_autotuner_scope() {
    autotuner_scope.clear();
    autotuner_input_params.clear();
    autotuner_output_params.clear();
  }
  
  void clear_state_scope() {
    state_local_scope.clear();
    state_input_params.clear();
  }
  
  bool is_variable_declared(const std::string& name) {
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
}

// Token declarations
%token <std::string> IDENTIFIER DOUBLE INTEGER STRING

%token AUTOTUNER ROUTINE STATE START REQUIRES TERMINAL IF ELSE TRUE FALSE NIL CONFIG_VAR
%token FLOAT_KW INT_KW BOOL_KW STRING_KW QUANTITY_KW CONFIG_KW CONNECTION_KW CONNECTIONS_KW GNAME_KW ERROR_KW
%token ARROW DOUBLECOLON LBRACKET RBRACKET LBRACE RBRACE LPAREN RPAREN ASSIGN COMMA COLON SEMICOLON DOT
%token PLUS MINUS MUL DIV EQ NE LL GG LE GE AND OR NOT

// Type declarations for grammar rules
%type <std::unique_ptr<Program>> program
%type <std::vector<AutotunerDecl>> autotuner_list
%type <std::unique_ptr<AutotunerDecl>> autotuner_decl
%type <std::vector<std::unique_ptr<ParamDecl>>> param_decl_list input_params output_params state_input_params
%type <std::unique_ptr<ParamDecl>> param_decl
%type <std::unique_ptr<TypeDescriptor>> type_spec
%type <std::vector<std::string>> requires_clause identifier_list
%type <std::vector<std::unique_ptr<Stmt>>> autotuner_var_decls
%type <std::unique_ptr<VarDeclStmt>> var_decl_stmt
%type <std::string> entry_state
%type <std::vector<std::unique_ptr<Expr>>> entry_params
%type <std::vector<StateDecl>> state_list
%type <std::unique_ptr<StateDecl>> state_decl
%type <std::vector<std::unique_ptr<Stmt>>> stmt_list
%type <std::unique_ptr<Stmt>> stmt
%type <std::unique_ptr<Expr>> expr primary_expr postfix_expr
%type <std::vector<std::unique_ptr<Expr>>> expr_list 
%type <std::vector<NamedArg>> named_arg_list 
%type <std::unique_ptr<NamedArg>> named_arg%type <std::vector<RoutineDecl>> routine_list
%type <std::unique_ptr<RoutineDecl>> routine_decl

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
    : autotuner_list[autotuners] routine_list[routines]
      {
        $result = std::make_unique<Program>();
        $result->autotuners = std::move($autotuners);
        $result->routines = std::move($routines);
        program_root = std::move($result);
      }
    ;


autotuner_list[result]
    : autotuner_decl[first_autotuner]
      {
        $result = std::vector<AutotunerDecl>();
        $result.push_back(std::move(*$first_autotuner));
      }
    | autotuner_list[existing_list] autotuner_decl[next_autotuner]
      {
        $result = std::move($existing_list);
        $result.push_back(std::move(*$next_autotuner));
      }
    ;

routine_list[result]
    : routine_decl[first_routine]
      {
        $result = std::vector<RoutineDecl>();
        $result.push_back(std::move(*$first_routine));
      }
    | routine_list[existing_routines] routine_decl[next_routine]
      {
        $result = std::move($existing_routines);
        $result.push_back(std::move(*$next_routine));
      }
    | %empty
      {
        $result = std::vector<RoutineDecl>();
      }
    ;

// ============================================================================
// ROUTINE DECLARATION
// ============================================================================

routine_decl[result]
    : ROUTINE IDENTIFIER[name] input_params[inputs] ARROW output_params[outputs]
      {
        $result = std::make_unique<RoutineDecl>(
          std::move($name),
          std::move($inputs),
          std::move($outputs)
        );
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
        requires_clause[requires]
        autotuner_var_decls[vars]
        entry_state[entry] entry_params[params] SEMICOLON
        state_list[states]
      RBRACE
      {
        $result = std::make_unique<AutotunerDecl>(
          std::move($name),
          std::move($inputs),
          std::move($outputs),
          std::move($requires),
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
        // Register input parameters in scope (read-only)
        for (const auto& param : $params) {
          if (autotuner_input_params.count(param->name) > 0) {
            error(@params, "Duplicate input parameter: " + param->name);
            YYABORT;
          }
          autotuner_input_params.insert(param->name);
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
    : param_decl[first_param]
      {
        $result = std::vector<std::unique_ptr<ParamDecl>>();
        $result.push_back(std::move($first_param));
      }
    | param_decl_list[existing_params] COMMA param_decl[next_param]
      {
        $result = std::move($existing_params);
        $result.push_back(std::move($next_param));
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
    | QUANTITY_KW      
      { $result = std::make_unique<TypeDescriptor>(ParamType::Quantity); }
    | CONFIG_KW        
      { $result = std::make_unique<TypeDescriptor>(ParamType::Config); }
    | CONNECTION_KW    
      { $result = std::make_unique<TypeDescriptor>(ParamType::Connection); }
    | CONNECTIONS_KW   
      { $result = std::make_unique<TypeDescriptor>(ParamType::Connections); }
    | GNAME_KW         
      { $result = std::make_unique<TypeDescriptor>(ParamType::Gname); }
    | ERROR_KW         
      { $result = std::make_unique<TypeDescriptor>(ParamType::Error, "Error"); }
    ;

// ============================================================================
// AUTOTUNER COMPONENTS
// ============================================================================

requires_clause[result]
    : REQUIRES COLON LBRACKET identifier_list[required_autotuners] RBRACKET SEMICOLON
      { 
        $result = std::move($required_autotuners); 
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
    : stmt[first_var]
      {
        $result = std::vector<std::unique_ptr<Stmt>>();
        $result.push_back(std::move($first_var));
      }
    | autotuner_var_decls[existing_vars] stmt[next_var]
      {
        $result = std::move($existing_vars);
        $result.push_back(std::move($next_var));
      }
    | %empty
      {
        $result = std::vector<std::unique_ptr<Stmt>>();
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
    : stmt[first_stmt]
      {
        $result = std::vector<std::unique_ptr<Stmt>>();
        $result.push_back(std::move($first_stmt));
      }
    | stmt_list[existing_stmts] stmt[next_stmt]
      {
        $result = std::move($existing_stmts);
        $result.push_back(std::move($next_stmt));
      }
    | %empty
      {
        $result = std::vector<std::unique_ptr<Stmt>>();
      }
    ;

stmt[result]
    : var_decl_stmt[var_decl]
      { 
        $result = std::move($var_decl); 
      }
    | identifier_list[targets] ASSIGN expr[value] SEMICOLON
      {
        // Validate that all targets are declared
        for (const auto& target : $targets) {
          if (!is_variable_declared(target)) {
            error(@targets, "Undeclared variable in assignment: " + target);
            YYABORT;
          }
          // Check if trying to assign to input parameter (read-only)
          if (autotuner_input_params.count(target) > 0 || state_input_params.count(target) > 0) {
            error(@targets, "Cannot assign to read-only input parameter: " + target);
            YYABORT;
          }
        }
        
        $result = std::make_unique<AssignStmt>(
          std::move($targets), 
          std::move($value)
        );
      }
    | expr[side_effect_expr] SEMICOLON
      {
        $result = std::make_unique<ExprStmt>(std::move($side_effect_expr));
      }
    | IF LPAREN expr[condition] RPAREN LBRACE stmt_list[then_body] RBRACE
      {
        $result = std::make_unique<IfStmt>(
          std::move($condition), 
          std::move($then_body)
        );
      }
    | IF LPAREN expr[condition] RPAREN LBRACE stmt_list[then_body] RBRACE 
      ELSE LBRACE stmt_list[else_body] RBRACE
      {
        $result = std::make_unique<IfStmt>(
          std::move($condition), 
          std::move($then_body), 
          std::move($else_body)
        );
      }
    | ARROW IDENTIFIER[target_state] SEMICOLON
      {
        $result = std::make_unique<TransitionStmt>(
          std::move($target_state)
        );
      }
    | ARROW IDENTIFIER[target_state] LPAREN expr_list[params] RPAREN SEMICOLON
      {
        $result = std::make_unique<TransitionStmt>(
          std::move($target_state), 
          std::move($params)
        );
      }
    | TERMINAL SEMICOLON
      {
        $result = std::make_unique<TerminalStmt>();
      }
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
    | IDENTIFIER[qualifier] DOUBLECOLON IDENTIFIER[function_name] LPAREN expr_list[positional_args] RPAREN
      { 
        $result = std::make_unique<QualifiedCallExpr>(
          std::move($qualifier), 
          std::move($function_name), 
          std::move($positional_args), 
          std::vector<NamedArg>()
        ); 
      }
    | IDENTIFIER[qualifier] DOUBLECOLON IDENTIFIER[function_name] LPAREN named_arg_list[named_args] RPAREN
      { 
        $result = std::make_unique<QualifiedCallExpr>(
          std::move($qualifier), 
          std::move($function_name), 
          std::vector<std::unique_ptr<Expr>>(), 
          std::move($named_args)
        ); 
      }
    | IDENTIFIER[function_name] LPAREN expr_list[args] RPAREN
      { 
        $result = std::make_unique<CallExpr>(
          std::move($function_name), 
          std::move($args)
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
    | CONFIG_VAR
      { 
        $result = std::make_unique<VarExpr>("config"); 
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

named_arg_list[result]
    : named_arg[first_arg]
      {
        $result = std::vector<NamedArg>();
        $result.push_back(std::move(*$first_arg));
      }
    | named_arg_list[existing_args] COMMA named_arg[next_arg]
      {
        $result = std::move($existing_args);
        $result.push_back(std::move(*$next_arg));
      }
    ;

named_arg[result]
    : IDENTIFIER[param_name] ASSIGN expr[param_value]
      {
        $result = std::make_unique<NamedArg>(std::move($param_name), std::move($param_value));
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
