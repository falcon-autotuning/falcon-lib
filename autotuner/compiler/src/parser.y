%{
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "falcon-atc/AST.hpp"

using namespace falcon::atc;

extern int yylex();
extern int yylineno;
void yyerror(const char *s) { std::cerr << "Error at line " << yylineno << ": " << s << std::endl; }

Program* program_root;

static std::vector<std::unique_ptr<Expr>> to_unique_vec(std::vector<Expr*>* v) {
    std::vector<std::unique_ptr<Expr>> res;
    if (v) {
        for (auto e : *v) res.push_back(std::unique_ptr<Expr>(e));
    }
    return res;
}

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

program : autotuners { program_root = new Program{$1 ? std::move(*$1) : std::vector<AutotunerDecl>{}}; delete $1; }
        ;

autotuners : autotuner_decl { $$ = new std::vector<AutotunerDecl>(); $$->push_back(std::move(*$1)); delete $1; }
           | autotuners autotuner_decl { $1->push_back(std::move(*$2)); $$ = $1; delete $2; }
           ;

autotuner_decl : TAUTOTUNER TIDENTIFIER input_params generic_params TARROW output_params TLBRACE requires_clause spec_inputs spec_outputs params_block entry_clause loop_list states TRBRACE
                 { $$ = new AutotunerDecl{*$2, std::move(*$3), std::move(*$6), std::move(*$4), std::move(*$9), std::move(*$10), std::move(*$8), std::move(*$11), *$12, std::move(*$14), std::move(*$13)}; 
                   delete $2; delete $3; delete $4; delete $6; delete $8; delete $9; delete $10; delete $11; delete $12; delete $13; delete $14; }
               ;

input_params : TLPAREN sig_param_list TRPAREN { $$ = $2; }
             | { $$ = new std::vector<ParamDecl>(); }
             ;

output_params : TLPAREN sig_param_list TRPAREN { $$ = $2; }
              | { $$ = new std::vector<ParamDecl>(); }
              ;

sig_param_list : sig_param_decl { $$ = new std::vector<ParamDecl>(); $$->push_back(std::move(*$1)); delete $1; }
               | sig_param_list TCOMMA sig_param_decl { $1->push_back(std::move(*$3)); $$ = $1; delete $3; }
               | { $$ = new std::vector<ParamDecl>(); }
               ;

sig_param_decl : type_spec TIDENTIFIER { $$ = new ParamDecl{*$2, (ParamType)$1, nullptr}; delete $2; }
               ;

generic_params : TLBRACKET separated_idents TRBRACKET { $$ = $2; }
               | { $$ = new std::vector<std::string>(); }
               ;

spec_inputs : TSPEC_INPUTS TCOLON TLBRACKET spec_list TRBRACKET { $$ = $4; }
            | { $$ = new std::vector<SpecDecl>(); }
            ;

spec_outputs : TSPEC_OUTPUTS TCOLON TLBRACKET spec_list TRBRACKET { $$ = $4; }
             | { $$ = new std::vector<SpecDecl>(); }
             ;

spec_list : spec_decl { $$ = new std::vector<SpecDecl>(); $$->push_back(std::move(*$1)); delete $1; }
          | spec_list spec_decl { $1->push_back(std::move(*$2)); $$ = $1; delete $2; }
          | { $$ = new std::vector<SpecDecl>(); }
          ;

spec_decl : type_spec TIDENTIFIER TLBRACKET TIDENTIFIER TRBRACKET
            { $$ = new SpecDecl{(ParamType)$1, *$2, *$4}; delete $2; delete $4; }
          | type_spec TIDENTIFIER
            { $$ = new SpecDecl{(ParamType)$1, *$2, ""}; delete $2; }
          ;

requires_clause : TREQUIRES TCOLON TLBRACKET separated_strings TRBRACKET TSEMICOLON { $$ = $4; }
                | { $$ = new std::vector<std::string>(); }
                ;

separated_idents : TIDENTIFIER { $$ = new std::vector<std::string>(); $$->push_back(std::move(*$1)); delete $1; }
                 | separated_idents TCOMMA TIDENTIFIER { $1->push_back(std::move(*$3)); $$ = $1; delete $3; }
                 ;

separated_strings : TSTRING { $$ = new std::vector<std::string>(); $$->push_back(std::move(*$1)); delete $1; }
                  | TIDENTIFIER { $$ = new std::vector<std::string>(); $$->push_back(std::move(*$1)); delete $1; }
                  | separated_strings TCOMMA TSTRING { $1->push_back(std::move(*$3)); $$ = $1; delete $3; }
                  | separated_strings TCOMMA TIDENTIFIER { $1->push_back(std::move(*$3)); $$ = $1; delete $3; }
                  ;

params_block : TPARAMS TLBRACE param_list TRBRACE { $$ = $3; }
             | { $$ = new std::vector<ParamDecl>(); }
             ;

param_list : param_decl { $$ = new std::vector<ParamDecl>(); $$->push_back(std::move(*$1)); delete $1; }
           | param_list param_decl { $1->push_back(std::move(*$2)); $$ = $1; delete $2; }
           ;

param_decl : type_spec TIDENTIFIER TSEMICOLON 
             { $$ = new ParamDecl{*$2, (ParamType)$1, nullptr}; delete $2; }
           | type_spec TIDENTIFIER TASSIGN expr TSEMICOLON
             { /* simplified: only handling literal defaults for now */
               $$ = new ParamDecl{*$2, (ParamType)$1, nullptr}; delete $2; delete $4; }
           ;

type_spec : TFLOAT_KW { $$ = (int)ParamType::Float; }
          | TINT_KW { $$ = (int)ParamType::Int; }
          | TBOOL_KW { $$ = (int)ParamType::Bool; }
          | TSTRING_KW { $$ = (int)ParamType::String; }
          | TQUANTITY_KW { $$ = (int)ParamType::Quantity; }
          | TCONFIG_KW { $$ = (int)ParamType::Config; }
          | TGROUP_KW { $$ = (int)ParamType::Group; }
          | TCONNECTION_KW { $$ = (int)ParamType::Connection; }
          ;

entry_clause : TSTART TARROW TIDENTIFIER TSEMICOLON { $$ = $3; }
             ;

loop_list : loop_decl { $$ = new std::vector<ForLoop>(); $$->push_back(std::move(*$1)); delete $1; }
          | loop_list loop_decl { $1->push_back(std::move(*$2)); $$ = $1; delete $2; }
          | { $$ = new std::vector<ForLoop>(); }
          ;

loop_decl : TFOR TIDENTIFIER TIN expr TLBRACE loop_states TRBRACE
            { $$ = new ForLoop{*$2, std::unique_ptr<Expr>($4), std::move(*$6)}; delete $2; delete $6; }
          ;

loop_states : state_decl { $$ = new std::vector<StateDecl>(); $$->push_back(std::move(*$1)); delete $1; }
            | loop_states state_decl { $1->push_back(std::move(*$2)); $$ = $1; delete $2; }
            ;

states : state_list { $$ = $1; }
       ;

state_list : state_decl { $$ = new std::vector<StateDecl>(); $$->push_back(std::move(*$1)); delete $1; }
           | state_list state_decl { $1->push_back(std::move(*$2)); $$ = $1; delete $2; }
           | { $$ = new std::vector<StateDecl>(); }
           ;

state_decl : TSTATE TIDENTIFIER TLBRACKET TIDENTIFIER TRBRACKET TLBRACE state_params state_temps measurement_opt transition_list TRBRACE
             { $$ = new StateDecl{*$2, *$4, std::move(*$7), std::move(*$8), std::unique_ptr<Expr>($9), false, std::move(*$10)}; 
               delete $2; delete $4; delete $7; delete $8; delete $10; }
           | TSTATE TIDENTIFIER TLBRACE state_params state_temps measurement_opt transition_list TRBRACE
             { $$ = new StateDecl{*$2, "", std::move(*$4), std::move(*$5), std::unique_ptr<Expr>($6), false, std::move(*$7)}; 
               delete $2; delete $4; delete $5; delete $7; }
           ;

state_params : TPARAMS TLBRACE param_list TRBRACE { $$ = $3; }
             | { $$ = new std::vector<ParamDecl>(); }
             ;

state_temps : TTEMP TLBRACE param_list TRBRACE { $$ = $3; }
            | { $$ = new std::vector<ParamDecl>(); }
            ;

measurement_opt : TMEASUREMENT TCOLON TIDENTIFIER TLPAREN arg_list TRPAREN TSEMICOLON { $$ = new CallExpr{*$3, to_unique_vec($5)}; delete $3; delete $5; }
                | TRUN TCOLON TIDENTIFIER TLPAREN arg_list TRPAREN TSEMICOLON { $$ = new CallExpr{*$3, to_unique_vec($5)}; delete $3; delete $5; }
                | { $$ = nullptr; }
                ;

arg_list : expr { $$ = new std::vector<Expr*>(); $$->push_back($1); }
         | arg_list TCOMMA expr { $1->push_back($3); $$ = $1; }
         | { $$ = new std::vector<Expr*>(); }
         ;

transition_list : transition_decl { $$ = $1; }
                | transition_list transition_decl { $1->insert($1->end(), std::make_move_iterator($2->begin()), std::make_move_iterator($2->end())); delete $2; $$ = $1; }
                | { $$ = new std::vector<Transition>(); }
                ;

transition_decl : simple_transition { $$ = $1; }
                | TIF TLPAREN expr TRPAREN transition_decl
                  { 
                    $$ = $5;
                    for (auto& t : *$$) {
                      if (t.condition) t.condition = std::make_unique<BinaryExpr>("&&", $3->clone(), std::move(t.condition));
                      else t.condition = $3->clone();
                    }
                    delete $3;
                  }
                | TIF TLPAREN expr TRPAREN simple_transition TELSE transition_decl
                  {
                    $$ = $5;
                    for (auto& t : *$$) {
                        if (t.condition) t.condition = std::make_unique<BinaryExpr>("&&", $3->clone(), std::move(t.condition));
                        else t.condition = $3->clone();
                    }
                    auto else_part = $7;
                    auto inv_cond = std::make_unique<UnaryExpr>("!", $3->clone());
                    for (auto& t : *else_part) {
                        if (t.condition) t.condition = std::make_unique<BinaryExpr>("&&", inv_cond->clone(), std::move(t.condition));
                        else t.condition = inv_cond->clone();
                    }
                    $$->insert($$->end(), std::make_move_iterator(else_part->begin()), std::make_move_iterator(else_part->end()));
                    delete else_part;
                    delete $3;
                  }
                ;

simple_transition : TSUCCESS TSEMICOLON
                  { $$ = new std::vector<Transition>(); Transition t; t.is_success = true; $$->push_back(std::move(t)); }
                | TFAIL TSTRING TSEMICOLON
                  { $$ = new std::vector<Transition>(); Transition t; t.is_fail = true; t.error_message = *$2; $$->push_back(std::move(t)); delete $2; }
                | TFAIL TSEMICOLON
                  { $$ = new std::vector<Transition>(); Transition t; t.is_fail = true; $$->push_back(std::move(t)); }
                | TTERMINAL TSEMICOLON
                  { $$ = new std::vector<Transition>(); Transition t; t.target.state_name = "_TERMINAL_"; $$->push_back(std::move(t)); }
                | TARROW trans_target TSEMICOLON
                  { $$ = new std::vector<Transition>(); $$->emplace_back(nullptr, std::move(*$2), std::vector<Assignment>{}); delete $2; }
                | assignment_block TARROW trans_target TSEMICOLON
                  { $$ = new std::vector<Transition>(); $$->emplace_back(nullptr, std::move(*$3), std::move(*$1)); delete $1; delete $3; }
                | assignment_decl
                  { 
                    $$ = new std::vector<Transition>();
                    std::vector<Assignment> asgns;
                    asgns.push_back(std::move(*$1));
                    $$->emplace_back(nullptr, TransitionTarget("", "", nullptr, {}), std::move(asgns));
                    delete $1;
                  }
                | TLBRACE transition_list TRBRACE
                  {
                    $$ = $2;
                  }
                ;

assignment_block : TLBRACE assignment_list TRBRACE { $$ = $2; }
                 | { $$ = new std::vector<Assignment>(); }
                 ;

assignment_list : assignment_decl { $$ = new std::vector<Assignment>(); $$->push_back(std::move(*$1)); delete $1; }
                | assignment_list assignment_decl { $1->push_back(std::move(*$2)); $$ = $1; delete $2; }
                ;

assignment_decl : separated_idents TASSIGN expr TSEMICOLON { $$ = new Assignment{std::move(*$1), std::unique_ptr<Expr>($3)}; delete $1; }
                ;

trans_target : TIDENTIFIER TLBRACKET expr TRBRACKET mappings { $$ = new TransitionTarget{"", std::move(*$1), std::unique_ptr<Expr>($3), std::move(*$5)}; delete $1; delete $5; }
             | TIDENTIFIER mappings { $$ = new TransitionTarget{"", std::move(*$1), nullptr, std::move(*$2)}; delete $1; delete $2; }
             | TIDENTIFIER TDOUBLECOLON TIDENTIFIER TLBRACKET expr TRBRACKET mappings { $$ = new TransitionTarget{std::move(*$1), std::move(*$3), std::unique_ptr<Expr>($5), std::move(*$7)}; delete $1; delete $3; delete $7; }
             | TIDENTIFIER TDOUBLECOLON TIDENTIFIER mappings { $$ = new TransitionTarget{std::move(*$1), std::move(*$3), nullptr, std::move(*$4)}; delete $1; delete $3; delete $4; }
             ;

mappings : TLBRACKET mapping_list TRBRACKET { $$ = $2; }
         | { $$ = new std::vector<Mapping>(); }
         ;

mapping_list : mapping_decl { $$ = new std::vector<Mapping>(); $$->push_back(std::move(*$1)); delete $1; }
             | mapping_list TCOMMA mapping_decl { $1->push_back(std::move(*$3)); $$ = $1; delete $3; }
             ;

mapping_decl : TIDENTIFIER TCOLON TIDENTIFIER { $$ = new Mapping{*$1, *$3}; delete $1; delete $3; }
             | TIDENTIFIER { $$ = new Mapping{*$1, *$1}; delete $1; }
             ;

expr : TINTEGER { $$ = new ConstExpr{std::stoll(*$1)}; delete $1; }
     | TDOUBLE { $$ = new ConstExpr{std::stod(*$1)}; delete $1; }
     | TSTRING { $$ = new ConstExpr{*$1}; delete $1; }
     | TTRUE { $$ = new ConstExpr{true}; }
     | TFALSE { $$ = new ConstExpr{false}; }
     | TIDENTIFIER { $$ = new VarExpr{*$1}; delete $1; }
     | TCONFIG_VAR { $$ = new VarExpr{"config"}; }
     | TNEXT TIDENTIFIER { $$ = new VarExpr("next " + *$2); delete $2; }
     | expr TDOT TIDENTIFIER { $$ = new MemberExpr(std::unique_ptr<Expr>($1), *$3); delete $3; }
     | expr TPLUS expr { $$ = new BinaryExpr("+", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TMINUS expr { $$ = new BinaryExpr("-", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TMUL expr { $$ = new BinaryExpr("*", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TDIV expr { $$ = new BinaryExpr("/", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TEQ expr { $$ = new BinaryExpr("==", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TNE expr { $$ = new BinaryExpr("!=", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TLL expr { $$ = new BinaryExpr("<", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TGG expr { $$ = new BinaryExpr(">", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TLE expr { $$ = new BinaryExpr("<=", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TGE expr { $$ = new BinaryExpr(">=", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TAND expr { $$ = new BinaryExpr("&&", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | expr TOR expr { $$ = new BinaryExpr("||", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
     | TNOT expr { $$ = new UnaryExpr("!", std::unique_ptr<Expr>($2)); }
     | TMINUS expr %prec TNOT { $$ = new UnaryExpr("-", std::unique_ptr<Expr>($2)); }
     | TLPAREN expr TRPAREN { $$ = $2; }
     ;

%%
