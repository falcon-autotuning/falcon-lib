%require "3.8"
%language "c++"

%define api.namespace {falcon::atc}
%define api.parser.class {Parser}
%define api.value.type variant
%define api.token.constructor
%define api.token.prefix {TOK_}
%define parse.error verbose
%define parse.trace
%locations  // ← ADD THIS LINE

// Enable tracing and verbose errors (which may be wrong!)
%code requires {
  #include <string>
  #include <vector>
  #include <memory>
  #include "falcon-atc/AST.hpp"
  #include "falcon-atc/ParseError.hpp"  // ← ADD THIS LINE
}

%code {
  #include <iostream>
  
  // This will be defined in lexer
  namespace falcon::atc {
    Parser::symbol_type yylex();
  }
  
  std::unique_ptr<falcon::atc::Program> program_root;
}

// Token declarations
%token <std::string> IDENTIFIER DOUBLE INTEGER STRING

%token AUTOTUNER STATE PARAMS TEMP MEASUREMENT RUN START REQUIRES TERMINAL IF ELSE TRUE FALSE
%token SUCCESS FAIL SPEC_INPUTS SPEC_OUTPUTS CONFIG_VAR NEXT FOR IN
%token FLOAT_KW INT_KW BOOL_KW STRING_KW QUANTITY_KW CONFIG_KW GROUP_KW CONNECTION_KW
%token ARROW DOUBLECOLON LBRACKET RBRACKET LBRACE RBRACE LPAREN RPAREN ASSIGN COMMA COLON SEMICOLON DOT
%token PLUS MINUS MUL DIV EQ NE LL GG LE GE AND OR NOT

// Type declarations
%type <std::unique_ptr<Program>> program
%type <std::vector<AutotunerDecl>> autotuners
%type <std::unique_ptr<AutotunerDecl>> autotuner_decl
%type <std::vector<std::string>> requires_clause generic_params separated_idents separated_strings
%type <std::vector<ParamDecl>> params_block param_list input_params output_params sig_param_list state_temps
%type <std::unique_ptr<ParamDecl>> param_decl sig_param_decl
%type <ParamType> type_spec
%type <std::string> entry_clause
%type <std::vector<StateDecl>> states state_list loop_states
%type <std::unique_ptr<StateDecl>> state_decl
%type <std::vector<std::unique_ptr<Expr>>> arg_list nonempty_arg_list
%type <std::vector<Transition>> transition_list transition_decl simple_transition
%type <std::vector<Assignment>> assignment_list
%type <std::unique_ptr<TransitionTarget>> trans_target
%type <std::vector<Mapping>> mappings mapping_list
%type <std::unique_ptr<Mapping>> mapping_decl
%type <std::unique_ptr<Expr>> expr primary_expr measurement_opt
%type <std::vector<SpecDecl>> spec_inputs spec_outputs spec_list
%type <std::unique_ptr<SpecDecl>> spec_decl
%type <std::vector<ForLoop>> loop_list
%type <std::unique_ptr<ForLoop>> loop_decl

%left OR
%left AND
%left EQ NE LL GG LE GE
%left PLUS MINUS
%left MUL DIV
%right NOT UMINUS
%left DOT
%left LPAREN

%start program

%%

program[result]
    : autotuners[at] 
      { 
        $result = std::make_unique<Program>();
        $result->autotuners = std::move($at);
        program_root = std::move($result);
      }
    ;

autotuners[result]
    : autotuner_decl[decl] 
      { 
        $result = std::vector<AutotunerDecl>();
        $result.push_back(std::move(*$decl));
      }
    | autotuners[list] autotuner_decl[decl] 
      { 
        $result = std::move($list);
        $result.push_back(std::move(*$decl));
      }
    ;

autotuner_decl[result]
    : AUTOTUNER IDENTIFIER[name] 
      input_params[inputs] 
      generic_params[generics] 
      ARROW 
      output_params[outputs] 
      LBRACE 
        requires_clause[requires]
        spec_inputs[spec_in]
        spec_outputs[spec_out]
        params_block[params]
        entry_clause[entry]
        loop_list[loops]
        states[state_list]
      RBRACE 
      { 
        $result = std::make_unique<AutotunerDecl>(
          std::move($name),
          std::move($inputs),
          std::move($outputs),
          std::move($generics),
          std::move($spec_in),
          std::move($spec_out),
          std::move($requires),
          std::move($params),
          std::move($entry),
          std::move($state_list),
          std::move($loops)
        );
      }
    ;

input_params[result]
    : LPAREN sig_param_list[params] RPAREN 
      { $result = std::move($params); }
    | %empty
      { $result = std::vector<ParamDecl>(); }
    ;

output_params[result]
    : LPAREN sig_param_list[params] RPAREN 
      { $result = std::move($params); }
    | %empty
      { $result = std::vector<ParamDecl>(); }
    ;

sig_param_list[result]
    : sig_param_decl[decl]
      { 
        $result = std::vector<ParamDecl>();
        $result.push_back(std::move(*$decl));
      }
    | sig_param_list[list] COMMA sig_param_decl[decl]
      { 
        $result = std::move($list);
        $result.push_back(std::move(*$decl));
      }
    | %empty
      { $result = std::vector<ParamDecl>(); }
    ;

sig_param_decl[result]
    : type_spec[type] IDENTIFIER[name]
      { 
        $result = std::make_unique<ParamDecl>();
        $result->name = std::move($name);
        $result->type = $type;
        $result->default_value = nullptr;
      }
    ;

generic_params[result]
    : LBRACKET separated_idents[idents] RBRACKET 
      { $result = std::move($idents); }
    | %empty
      { $result = std::vector<std::string>(); }
    ;

spec_inputs[result]
    : SPEC_INPUTS COLON LBRACKET spec_list[list] RBRACKET 
      { $result = std::move($list); }
    | %empty
      { $result = std::vector<SpecDecl>(); }
    ;

spec_outputs[result]
    : SPEC_OUTPUTS COLON LBRACKET spec_list[list] RBRACKET 
      { $result = std::move($list); }
    | %empty
      { $result = std::vector<SpecDecl>(); }
    ;

spec_list[result]
    : spec_decl[decl]
      { 
        $result = std::vector<SpecDecl>();
        $result.push_back(std::move(*$decl));
      }
    | spec_list[list] spec_decl[decl]
      { 
        $result = std::move($list);
        $result.push_back(std::move(*$decl));
      }
    | %empty
      { $result = std::vector<SpecDecl>(); }
    ;

spec_decl[result]
    : type_spec[type] IDENTIFIER[name] LBRACKET IDENTIFIER[param] RBRACKET
      { 
        $result = std::make_unique<SpecDecl>(
          $type, 
          std::move($name), 
          std::move($param)
        );
      }
    | type_spec[type] IDENTIFIER[name]
      { 
        $result = std::make_unique<SpecDecl>(
          $type, 
          std::move($name), 
          ""
        );
      }
    ;

requires_clause[result]
    : REQUIRES COLON LBRACKET separated_strings[strings] RBRACKET SEMICOLON 
      { $result = std::move($strings); }
    | %empty
      { $result = std::vector<std::string>(); }
    ;

separated_idents[result]
    : IDENTIFIER[id]
      { 
        $result = std::vector<std::string>();
        $result.push_back(std::move($id));
      }
    | separated_idents[list] COMMA IDENTIFIER[id]
      { 
        $result = std::move($list);
        $result.push_back(std::move($id));
      }
    ;

separated_strings[result]
    : STRING[str]
      { 
        $result = std::vector<std::string>();
        $result.push_back(std::move($str));
      }
    | IDENTIFIER[id]
      { 
        $result = std::vector<std::string>();
        $result.push_back(std::move($id));
      }
    | separated_strings[list] COMMA STRING[str]
      { 
        $result = std::move($list);
        $result.push_back(std::move($str));
      }
    | separated_strings[list] COMMA IDENTIFIER[id]
      { 
        $result = std::move($list);
        $result.push_back(std::move($id));
      }
    ;

params_block[result]
    : PARAMS LBRACE param_list[list] RBRACE 
      { $result = std::move($list); }
    | %empty
      { $result = std::vector<ParamDecl>(); }
    ;

param_list[result]
    : param_decl[decl]
      { 
        $result = std::vector<ParamDecl>();
        $result.push_back(std::move(*$decl));
      }
    | param_list[list] param_decl[decl]
      { 
        $result = std::move($list);
        $result.push_back(std::move(*$decl));
      }
    ;

param_decl[result]
    : type_spec[type] IDENTIFIER[name] SEMICOLON 
      { 
        $result = std::make_unique<ParamDecl>();
        $result->name = std::move($name);
        $result->type = $type;
        $result->default_value = nullptr;
      }
    // FIX: default value can come from spec too
    | type_spec[type] IDENTIFIER[name] ASSIGN expr[default_val] SEMICOLON 
      { 
        $result = std::make_unique<ParamDecl>();
        $result->name = std::move($name);
        $result->type = $type;
        $result->default_value = std::move($default_val);
      }
    ;

type_spec[result]
    : FLOAT_KW    { $result = ParamType::Float; }
    | INT_KW      { $result = ParamType::Int; }
    | BOOL_KW     { $result = ParamType::Bool; }
    | STRING_KW   { $result = ParamType::String; }
    | QUANTITY_KW { $result = ParamType::Quantity; }
    | CONFIG_KW   { $result = ParamType::Config; }
    | GROUP_KW    { $result = ParamType::Group; }
    | CONNECTION_KW { $result = ParamType::Connection; }
    ;

entry_clause[result]
    : START ARROW IDENTIFIER[state] SEMICOLON 
      { $result = std::move($state); }
    ;

loop_list[result]
    : loop_decl[decl]
      { 
        $result = std::vector<ForLoop>();
        $result.push_back(std::move(*$decl));
      }
    | loop_list[list] loop_decl[decl]
      { 
        $result = std::move($list);
        $result.push_back(std::move(*$decl));
      }
    | %empty
      { $result = std::vector<ForLoop>(); }
    ;

loop_decl[result]
    : FOR IDENTIFIER[var] IN expr[iterable] LBRACE loop_states[states] RBRACE 
      { 
        $result = std::make_unique<ForLoop>(
          std::move($var),
          std::move($iterable),
          std::move($states)
        );
      }
    ;

loop_states[result]
    : state_decl[decl]
      { 
        $result = std::vector<StateDecl>();
        $result.push_back(std::move(*$decl));
      }
    | loop_states[list] state_decl[decl]
      { 
        $result = std::move($list);
        $result.push_back(std::move(*$decl));
      }
    ;

states[result]
    : state_list[list]
      { $result = std::move($list); }
    ;

state_list[result]
    : state_decl[decl]
      { 
        $result = std::vector<StateDecl>();
        $result.push_back(std::move(*$decl));
      }
    | state_list[list] state_decl[decl]
      { 
        $result = std::move($list);
        $result.push_back(std::move(*$decl));
      }
    | %empty
      { $result = std::vector<StateDecl>(); }
    ;

state_decl[result]
    : STATE IDENTIFIER[name] 
      LBRACKET IDENTIFIER[param] RBRACKET 
      LBRACE 
        params_block[params]
        state_temps[temps]
        measurement_opt[measurement]
        transition_list[transitions]
      RBRACE
      { 
        $result = std::make_unique<StateDecl>(
          std::move($name),
          std::move($param),
          std::move($params),
          std::move($temps),
          std::move($measurement),
          false,
          std::move($transitions)
        );
      }
    | STATE IDENTIFIER[name] 
      LBRACE 
        params_block[params]
        state_temps[temps]
        measurement_opt[measurement]
        transition_list[transitions]
      RBRACE
      { 
        $result = std::make_unique<StateDecl>(
          std::move($name),
          "",
          std::move($params),
          std::move($temps),
          std::move($measurement),
          false,
          std::move($transitions)
        );
      }
    ;

state_temps[result]
    : TEMP LBRACE param_list[list] RBRACE 
      { $result = std::move($list); }
    | %empty
      { $result = std::vector<ParamDecl>(); }
    ;

measurement_opt[result]
    : MEASUREMENT COLON IDENTIFIER[func] LPAREN arg_list[args] RPAREN SEMICOLON
      { 
        $result = std::make_unique<CallExpr>(
          std::move($func),
          std::move($args)
        );
      }
    | RUN COLON IDENTIFIER[func] LPAREN arg_list[args] RPAREN SEMICOLON
      { 
        $result = std::make_unique<CallExpr>(
          std::move($func),
          std::move($args)
        );
      }
    | %empty
      { $result = nullptr; }
    ;

// Separate arg_list to avoid conflicts with function call expressions
arg_list[result]
    : nonempty_arg_list[list]
      { $result = std::move($list); }
    | %empty
      { $result = std::vector<std::unique_ptr<Expr>>(); }
    ;

nonempty_arg_list[result]
    : expr[e]
      { 
        $result = std::vector<std::unique_ptr<Expr>>();
        $result.push_back(std::move($e));
      }
    | nonempty_arg_list[list] COMMA expr[e]
      { 
        $result = std::move($list);
        $result.push_back(std::move($e));
      }
    ;

transition_list[result]
    : transition_decl[trans]
      { $result = std::move($trans); }
    | transition_list[list] transition_decl[trans]
      { 
        auto tmp = std::move($list);
        // Manual move to avoid automove issues
        for (auto& t : $trans) {
          tmp.push_back(std::move(t));
        }
        $result = std::move(tmp);
      }
    | %empty
      { $result = std::vector<Transition>(); }
    ;

transition_decl[result]
    : simple_transition[trans]
      { $result = std::move($trans); }
    | IF LPAREN expr[cond] RPAREN transition_decl[then_part]
      { 
        // Store condition, then iterate
        auto cond_ptr = std::move($cond);
        auto tmp = std::move($then_part);
        for (auto &t : tmp) {
          if (t.condition) {
            t.condition = std::make_unique<BinaryExpr>(
              "&&", 
              cond_ptr->clone(), 
              std::move(t.condition)
            );
          } else {
            t.condition = cond_ptr->clone();
          }
        }
        $result = std::move(tmp);
      }
    | IF LPAREN expr[cond] RPAREN simple_transition[then_part] ELSE transition_decl[else_part]
      { 
        // Store all values first to avoid multiple accesses
        auto cond_ptr = std::move($cond);
        auto tmp_then = std::move($then_part);
        auto tmp_else = std::move($else_part);
        
        // Process then branch
        for (auto &t : tmp_then) {
          if (t.condition) {
            t.condition = std::make_unique<BinaryExpr>(
              "&&", 
              cond_ptr->clone(), 
              std::move(t.condition)
            );
          } else {
            t.condition = cond_ptr->clone();
          }
        }
        
        // Process else branch
        auto inv_cond = std::make_unique<UnaryExpr>("!", cond_ptr->clone());
        for (auto &t : tmp_else) {
          if (t.condition) {
            t.condition = std::make_unique<BinaryExpr>(
              "&&", 
              inv_cond->clone(), 
              std::move(t.condition)
            );
          } else {
            t.condition = inv_cond->clone();
          }
        }
        
        // Combine
        for (auto& t : tmp_else) {
          tmp_then.push_back(std::move(t));
        }
        $result = std::move(tmp_then);
      }
    ;

simple_transition[result]
    : SUCCESS SEMICOLON 
      { 
        $result = std::vector<Transition>();
        Transition t;
        t.is_success = true;
        $result.push_back(std::move(t));
      }
    | FAIL STRING[msg] SEMICOLON 
      { 
        $result = std::vector<Transition>();
        Transition t;
        t.is_fail = true;
        t.error_message = std::move($msg);
        $result.push_back(std::move(t));
      }
    | FAIL SEMICOLON 
      { 
        $result = std::vector<Transition>();
        Transition t;
        t.is_fail = true;
        $result.push_back(std::move(t));
      }
    | TERMINAL SEMICOLON 
      { 
        $result = std::vector<Transition>();
        Transition t;
        t.target.state_name = "_TERMINAL_";
        $result.push_back(std::move(t));
      }
    | ARROW trans_target[target] SEMICOLON 
      { 
        $result = std::vector<Transition>();
        $result.emplace_back(
          nullptr, 
          std::move(*$target), 
          std::vector<Assignment>()
        );
      }
    | LBRACE assignment_list[assigns] RBRACE ARROW trans_target[target] SEMICOLON 
      { 
        $result = std::vector<Transition>();
        $result.emplace_back(
          nullptr, 
          std::move(*$target), 
          std::move($assigns)
        );
      }
    | separated_idents[targets] ASSIGN expr[value] SEMICOLON
      { 
        // Single assignment without semicolon as transition
        $result = std::vector<Transition>();
        std::vector<Assignment> asgns;
        asgns.emplace_back(std::move($targets), std::move($value));
        $result.emplace_back(
          nullptr, 
          TransitionTarget("", "", nullptr, {}),
          std::move(asgns)
        );
      }
    | LBRACE transition_list[list] RBRACE 
      { $result = std::move($list); }
    ;

assignment_list[result]
    : separated_idents[targets] ASSIGN expr[value] SEMICOLON
      { 
        $result = std::vector<Assignment>();
        $result.emplace_back(std::move($targets), std::move($value));
      }
    | assignment_list[list] separated_idents[targets] ASSIGN expr[value] SEMICOLON
      { 
        $result = std::move($list);
        $result.emplace_back(std::move($targets), std::move($value));
      }
    ;


trans_target[result]
    : IDENTIFIER[state] LBRACKET expr[param] RBRACKET mappings[maps]
      { 
        $result = std::make_unique<TransitionTarget>(
          "",
          std::move($state),
          std::move($param),
          std::move($maps)
        );
      }
    | IDENTIFIER[state] mappings[maps]
      { 
        $result = std::make_unique<TransitionTarget>(
          "",
          std::move($state),
          nullptr,
          std::move($maps)
        );
      }
    | IDENTIFIER[ns] DOUBLECOLON IDENTIFIER[state] LBRACKET expr[param] RBRACKET mappings[maps]
      { 
        $result = std::make_unique<TransitionTarget>(
          std::move($ns),
          std::move($state),
          std::move($param),
          std::move($maps)
        );
      }
    | IDENTIFIER[ns] DOUBLECOLON IDENTIFIER[state] mappings[maps]
      { 
        $result = std::make_unique<TransitionTarget>(
          std::move($ns),
          std::move($state),
          nullptr,
          std::move($maps)
        );
      }
    ;

mappings[result]
    : LBRACKET mapping_list[list] RBRACKET 
      { $result = std::move($list); }
    | %empty
      { $result = std::vector<Mapping>(); }
    ;

mapping_list[result]
    : mapping_decl[map]
      { 
        $result = std::vector<Mapping>();
        $result.push_back(std::move(*$map));
      }
    | mapping_list[list] COMMA mapping_decl[map]
      { 
        $result = std::move($list);
        $result.push_back(std::move(*$map));
      }
    ;

mapping_decl[result]
    : IDENTIFIER[src] COLON IDENTIFIER[dst]
      { 
        $result = std::make_unique<Mapping>(
          std::move($src),
          std::move($dst)
        );
      }
    | IDENTIFIER[name]
      { 
        // Need to copy before move since we use it twice
        std::string name_copy = $name;
        $result = std::make_unique<Mapping>(
          std::move($name),
          std::move(name_copy)
        );
      }
    ;

// Expression grammar - restructured to avoid conflicts
expr[result]
    : expr[left] PLUS expr[right]
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
    | expr[obj] DOT IDENTIFIER[member]
      { 
        $result = std::make_unique<MemberExpr>(std::move($obj), std::move($member));
      }
    | primary_expr[prim]
      { $result = std::move($prim); }
    ;

primary_expr[result]
    : INTEGER[val]
      { $result = std::make_unique<ConstExpr>(std::stoll($val)); }
    | DOUBLE[val]
      { $result = std::make_unique<ConstExpr>(std::stod($val)); }
    | STRING[val]
      { $result = std::make_unique<ConstExpr>(std::move($val)); }
    | TRUE
      { $result = std::make_unique<ConstExpr>(true); }
    | FALSE
      { $result = std::make_unique<ConstExpr>(false); }
    | IDENTIFIER[name]
      { $result = std::make_unique<VarExpr>(std::move($name)); }
    | CONFIG_VAR
      { $result = std::make_unique<VarExpr>("config"); }
    | NEXT IDENTIFIER[name]
      { $result = std::make_unique<VarExpr>("next " + $name); }
    | LPAREN expr[inner] RPAREN
      { $result = std::move($inner); }
    | IDENTIFIER[func] LPAREN arg_list[args] RPAREN
      { 
        $result = std::make_unique<CallExpr>(std::move($func), std::move($args));
      }
    ;

%%

void falcon::atc::Parser::error(const location_type& loc, const std::string& msg) {
  // Collect the error
  ParseError err{
    loc.begin.line,
    loc.begin.column,
    loc.end.line,
    loc.end.column,
    msg
  };
  
  current_errors.push_back(err);
}
