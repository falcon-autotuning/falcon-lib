%{
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "falcon-atc/AST.hpp"
#include "falcon-atc/ASTHelpers.hpp"

using namespace falcon::atc;

extern int yylex();
extern int yylineno;
void yyerror(const char *s) { std::cerr << "Error at line " << yylineno << ": " << s << std::endl; }

Program* program_root;


%}

%union {
    std::string* string;
    int token;
    falcon::atc::Program* program;
    falcon::atc::AutotunerDecl* autotuner;
    falcon::atc::StateDecl* state;
    falcon::atc::ParamDecl* param;
    falcon::atc::Expr* expr;
    std::vector<falcon::atc::AutotunerDecl>* autotuners;
    std::vector<falcon::atc::StateDecl>* states;
    std::vector<falcon::atc::ParamDecl>* params;
    std::vector<falcon::atc::Expr*>* exprs;
    std::vector<std::string>* strings;
    falcon::atc::Transition* transition;
    std::vector<falcon::atc::Transition>* transitions;
    falcon::atc::Assignment* assignment;
    std::vector<falcon::atc::Assignment>* assignments;
    falcon::atc::TransitionTarget* target;
    falcon::atc::Mapping* mapping;
    std::vector<falcon::atc::Mapping>* mappings;
    std::vector<falcon::atc::SpecDecl>* spec_decls;
    falcon::atc::SpecDecl* spec_decl;
    std::vector<falcon::atc::ForLoop>* loops;
    falcon::atc::ForLoop* loop;
}

%token <token> TAUTOTUNER TSTATE TPARAMS TTEMP TMEASUREMENT TRUN TSTART TREQUIRES TTERMINAL TIF TELSE TTRUE TFALSE
%token <token> TSUCCESS TFAIL TSPEC_INPUTS TSPEC_OUTPUTS TCONFIG_VAR TNEXT TFOR TIN
%token <token> TFLOAT_KW TINT_KW TBOOL_KW TSTRING_KW TQUANTITY_KW TCONFIG_KW TGROUP_KW TCONNECTION_KW
%token <token> TARROW TDOUBLECOLON TLBRACKET TRBRACKET TLBRACE TRBRACE TLPAREN TRPAREN TASSIGN TCOMMA TCOLON TSEMICOLON TDOT
%token <token> TPLUS TMINUS TMUL TDIV TEQ TNE TLL TGG TLE TGE TAND TOR TNOT
%token <string> TIDENTIFIER TDOUBLE TINTEGER TSTRING

%type <program> program
%type <autotuners> autotuners
%type <autotuner> autotuner_decl
%type <strings> requires_clause generic_params
%type <params> params_block param_list input_params output_params sig_param_list
%type <param> param_decl sig_param_decl
%type <token> type_spec
%type <string> entry_clause
%type <states> states state_list loop_states
%type <state> state_decl
%type <params> state_params state_temps
%type <exprs> arg_list
%type <transitions> transition_list transition_decl simple_transition
%type <assignments> assignment_block assignment_list
%type <assignment> assignment_decl
%type <target> trans_target
%type <mappings> mappings mapping_list
%type <mapping> mapping_decl
%type <expr> expr measurement_opt
%type <strings> separated_idents separated_strings
%type <spec_decls> spec_inputs spec_outputs spec_list
%type <spec_decl> spec_decl
%type <loops> loop_list
%type <loop> loop_decl

%left TOR
%left TAND
%left TEQ TNE TLL TGG TLE TGE
%left TPLUS TMINUS
%left TMUL TDIV
%nonassoc TNOT

%start program

%%

program 
    : autotuners 
      { program_root = Program::make($1); }
    ;

autotuners
    : autotuner_decl 
      { $$ = make_list<AutotunerDecl>(); $$ = append_item($$, $1); }
    | autotuners autotuner_decl 
      { $$ = append_item($1, $2); }
    ;

autotuner_decl 
    : TAUTOTUNER TIDENTIFIER input_params generic_params TARROW output_params TLBRACE requires_clause spec_inputs spec_outputs params_block entry_clause loop_list states TRBRACE 
      { $$ = AutotunerDecl::make($2, $3, $6, $4, $9, $10, $8, $11, $12, $13, $14); }
    ;

input_params 
    : TLPAREN sig_param_list TRPAREN 
      { $$ = $2; }
    | 
      { $$ = make_list<ParamDecl>(); }
    ;

output_params
    : TLPAREN sig_param_list TRPAREN 
      { $$ = $2; }
    | 
      { $$ = make_list<ParamDecl>(); }
    ;

sig_param_list
    : sig_param_decl 
      { $$ = make_list<ParamDecl>(); $$ = append_item($$, $1); }
    | sig_param_list TCOMMA sig_param_decl 
      { $$ = append_item($1, $3); }
    | 
      { $$ = make_list<ParamDecl>(); }
    ;

sig_param_decl
    : type_spec TIDENTIFIER 
      { $$ = ParamDecl::make($1, $2); }
    ;

generic_params 
    : TLBRACKET separated_idents TRBRACKET 
      { $$ = $2; }
    | 
      { $$ = make_list<std::string>(); }
    ;

spec_inputs 
    : TSPEC_INPUTS TCOLON TLBRACKET spec_list TRBRACKET 
      { $$ = $4; }
    | 
      { $$ = make_list<SpecDecl>(); }
    ;

spec_outputs 
    : TSPEC_OUTPUTS TCOLON TLBRACKET spec_list TRBRACKET 
      { $$ = $4; }
    | 
      { $$ = make_list<SpecDecl>(); }
    ;

spec_list 
    : spec_decl 
      { $$ = make_list<SpecDecl>(); $$ = append_item($$, $1); }
    | spec_list spec_decl 
      { $$ = append_item($1, $2); }
    | 
      { $$ = make_list<SpecDecl>(); }
    ;

spec_decl
    : type_spec TIDENTIFIER TLBRACKET TIDENTIFIER TRBRACKET
      { $$ = SpecDecl::make($1, $2, $4); }
    | type_spec TIDENTIFIER
      { $$ = SpecDecl::make($1, $2); }
    ;

requires_clause 
    : TREQUIRES TCOLON TLBRACKET separated_strings TRBRACKET TSEMICOLON 
      { $$ = $4; }
    | 
      { $$ = make_list<std::string>(); }
    ;

separated_idents 
    : TIDENTIFIER 
      { $$ = make_list<std::string>(); $$ = append_item($$, $1); }
    | separated_idents TCOMMA TIDENTIFIER 
      { $$ = append_item($1, $3); }
    ;

separated_strings 
    : TSTRING 
      { $$ = make_list<std::string>(); $$ = append_item($$, $1); }
    | TIDENTIFIER 
      { $$ = make_list<std::string>(); $$ = append_item($$, $1); }
    | separated_strings TCOMMA TSTRING 
      { $$ = append_item($1, $3); }
    | separated_strings TCOMMA TIDENTIFIER 
      { $$ = append_item($1, $3); }
    ;

params_block 
    : TPARAMS TLBRACE param_list TRBRACE 
      { $$ = $3; }
    | 
      { $$ = make_list<ParamDecl>(); }
    ;

param_list 
    : param_decl 
      { $$ = make_list<ParamDecl>(); $$ = append_item($$, $1); }
    | param_list param_decl 
      { $$ = append_item($1, $2); }
    ;

param_decl 
    : type_spec TIDENTIFIER TSEMICOLON 
      { $$ = ParamDecl::make($1,$2); }
    // Fix: only handling literal defaults for now
    | type_spec TIDENTIFIER TASSIGN expr TSEMICOLON 
      { $$ = ParamDecl::make($1,$2); delete $4; }
    ;

type_spec 
    : TFLOAT_KW 
      { $$ = (int)ParamType::Float; }
    | TINT_KW 
      { $$ = (int)ParamType::Int; }
    | TBOOL_KW 
      { $$ = (int)ParamType::Bool; }
    | TSTRING_KW 
      { $$ = (int)ParamType::String; }
    | TQUANTITY_KW 
      { $$ = (int)ParamType::Quantity; }
    | TCONFIG_KW 
      { $$ = (int)ParamType::Config; }
    | TGROUP_KW 
      { $$ = (int)ParamType::Group; }
    | TCONNECTION_KW 
      { $$ = (int)ParamType::Connection; }
    ;

entry_clause 
    : TSTART TARROW TIDENTIFIER TSEMICOLON 
      { $$ = $3; }
    ;

loop_list 
    : loop_decl 
      { $$ = make_list<ForLoop>(); $$ = append_item($$, $1); }
    | loop_list loop_decl 
      { $$ = append_item($1, $2); }
    | 
      { $$ = make_list<ForLoop>(); }
    ;

loop_decl 
    : TFOR TIDENTIFIER TIN expr TLBRACE loop_states TRBRACE 
      { $$ = ForLoop::make($2, $4, $6); }
    ;

loop_states 
    : state_decl 
      { $$ = make_list<StateDecl>(); $$ = append_item($$, $1); }
    | loop_states state_decl 
      { $$ = append_item($1,$2); }
    ;

states 
    : state_list 
      { $$ = $1; }
    ;

state_list 
    : state_decl 
      { $$ = make_list<StateDecl>(); $$ = append_item($$, $1); }
    | state_list state_decl 
      { $$ = append_item($1, $2); }
    | 
      { $$ = make_list<StateDecl>(); }
    ;

state_decl
    : TSTATE TIDENTIFIER TLBRACKET TIDENTIFIER TRBRACKET TLBRACE state_params state_temps measurement_opt transition_list TRBRACE
      { $$ = StateDecl::make($2, $7, $8, $9, $10, $4); }
    | TSTATE TIDENTIFIER TLBRACE state_params state_temps measurement_opt transition_list TRBRACE
      { $$ = StateDecl::make($2, $4, $5, $6, $7); }
    ;

state_params 
    : TPARAMS TLBRACE param_list TRBRACE 
      { $$ = $3; }
    | 
      { $$ = make_list<ParamDecl>(); }
    ;

state_temps 
    : TTEMP TLBRACE param_list TRBRACE 
      { $$ = $3; }
    | 
      { $$ = make_list<ParamDecl>(); }
    ;

measurement_opt
    : TMEASUREMENT TCOLON TIDENTIFIER TLPAREN arg_list TRPAREN TSEMICOLON
      { $$ = CallExpr::make($3, $5); }
    | TRUN TCOLON TIDENTIFIER TLPAREN arg_list TRPAREN TSEMICOLON
      { $$ = CallExpr::make($3, $5); }
    | 
      { $$ = nullptr; }
    ;

arg_list 
    : expr 
      { $$ = make_list<Expr*>(); $$ = append_item($$, $1); }
    | arg_list TCOMMA expr 
      { $$ = append_item($1, $3); }
    | 
      { $$ = make_list<Expr*>(); }
    ;

transition_list
    : transition_decl 
      { $$ = make_list<Transition>(); $$ = merge_lists($$, $1); }
    | transition_list transition_decl 
      { $$ = merge_lists($1, $2); }
    | 
      { $$ = make_list<Transition>(); }
    ;

transition_decl
    : simple_transition 
      { $$ = $1; }
    | TIF TLPAREN expr TRPAREN transition_decl
      { $$ = make_if_transition($3, $5); }
    | TIF TLPAREN expr TRPAREN simple_transition TELSE transition_decl
      { $$ = make_if_else_transition($3, $5, $7); }
    ;

simple_transition
    : TSUCCESS TSEMICOLON 
      { $$ = make_simple_transition(SimpleTransitionKind::Success); }
    | TFAIL TSTRING TSEMICOLON 
      { $$ = make_simple_transition(SimpleTransitionKind::Fail, $2); }
    | TFAIL TSEMICOLON 
      { $$ = make_simple_transition(SimpleTransitionKind::Fail); }
    | TTERMINAL TSEMICOLON 
      { $$ = make_simple_transition(SimpleTransitionKind::Terminal); }
    | TARROW trans_target TSEMICOLON 
      { $$ = make_arrow_transition($2); }
    | assignment_block TARROW trans_target TSEMICOLON 
      { $$ = make_assignment_arrow_transition($1, $3); }
    | assignment_decl 
      { $$ = make_assignment_transition($1); }
    | TLBRACE transition_list TRBRACE 
      { $$ = $2; }
    ;

assignment_block
    : TLBRACE assignment_list TRBRACE 
      { $$ = $2; }
    | 
      { $$ = make_list<Assignment>(); }
    ;

assignment_list
    : assignment_decl 
      { $$ = make_list<Assignment>(); $$ = append_item($$, $1); }
    | assignment_list assignment_decl 
      { $$ = append_item($1, $2); }
    ;

assignment_decl
    : separated_idents TASSIGN expr TSEMICOLON 
      { $$ = Assignment::make($1, $3); }
    ;

trans_target
    : TIDENTIFIER TLBRACKET expr TRBRACKET mappings
      { $$ = TransitionTarget::from_state_expr_mappings($1, $3, $5); }
    | TIDENTIFIER mappings
      { $$ = TransitionTarget::from_state_mappings($1, $2); }
    | TIDENTIFIER TDOUBLECOLON TIDENTIFIER TLBRACKET expr TRBRACKET mappings
      { $$ = TransitionTarget::from_ns_state_expr_mappings($1, $3, $5, $7); }
    | TIDENTIFIER TDOUBLECOLON TIDENTIFIER mappings
      { $$ = TransitionTarget::from_ns_state_mappings($1, $3, $4); }
    ;

mappings
    : TLBRACKET mapping_list TRBRACKET 
      { $$ = $2; }
    | 
      { $$ = make_list<Mapping>(); }
    ;

mapping_list
    : mapping_decl 
      { $$ = make_list<Mapping>(); $$ = append_item($$, $1); }
    | mapping_list TCOMMA mapping_decl 
      { $$ = append_item($1, $3); }
    ;

mapping_decl
    : TIDENTIFIER TCOLON TIDENTIFIER 
      { $$ = Mapping::make($1, $3); }
    | TIDENTIFIER 
      { $$ = Mapping::make($1); }
    ;

expr
    : TINTEGER 
      { $$ = ConstExpr::from_int($1); }
    | TDOUBLE 
      { $$ = ConstExpr::from_double($1); }
    | TSTRING 
      { $$ = ConstExpr::from_string($1); }
    | TTRUE 
      { $$ = ConstExpr::from_bool(true); }
    | TFALSE 
      { $$ = ConstExpr::from_bool(false); }
    | TIDENTIFIER 
      { $$ = VarExpr::make($1); }
    | TCONFIG_VAR 
      { $$ = VarExpr::make_config(); }
    | TNEXT TIDENTIFIER 
      { $$ = VarExpr::make_next($2); }
    | expr TDOT TIDENTIFIER 
      { $$ = new MemberExpr(std::unique_ptr<Expr>($1), *$3); delete $3; }
    | expr TPLUS expr 
      { $$ = BinaryExpr::make("+", $1, $3); }
    | expr TMINUS expr 
      { $$ = BinaryExpr::make("-", $1, $3); }
    | expr TMUL expr 
      { $$ = BinaryExpr::make("*", $1, $3); }
    | expr TDIV expr 
      { $$ = BinaryExpr::make("/", $1, $3); }
    | expr TEQ expr 
      { $$ = BinaryExpr::make("==", $1, $3); }
    | expr TNE expr 
      { $$ = BinaryExpr::make("!=", $1, $3); }
    | expr TLL expr 
      { $$ = BinaryExpr::make("<", $1, $3); }
    | expr TGG expr 
      { $$ = BinaryExpr::make(">", $1, $3); }
    | expr TLE expr 
      { $$ = BinaryExpr::make("<=", $1, $3); }
    | expr TGE expr 
      { $$ = BinaryExpr::make(">=", $1, $3); }
    | expr TAND expr 
      { $$ = BinaryExpr::make("&&", $1, $3); }
    | expr TOR expr 
      { $$ = BinaryExpr::make("||", $1, $3); }
    | TNOT expr 
      { $$ = UnaryExpr::make("!", $2); }
    | TMINUS expr %prec TNOT 
      { $$ = UnaryExpr::make("-", $2); }
    | TLPAREN expr TRPAREN 
      { $$ = $2; }
    ;
