%{
#include <iostream>
#include <string>
#include <vector>
#include "falcon-atc/AST.hpp"
#include "parser.tab.hpp"

#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)

extern int yylex();
%}

%option noyywrap
%option yylineno

%%

[ \t]                   ;
\n                      ;
"//".*                  ;

"float"                 return TOKEN(TFLOAT_KW);
"int"                   return TOKEN(TINT_KW);
"bool"                  return TOKEN(TBOOL_KW);
"string"                return TOKEN(TSTRING_KW);
"Quantity"              return TOKEN(TQUANTITY_KW);
"Config"                return TOKEN(TCONFIG_KW);
"Group"                 return TOKEN(TGROUP_KW);
"Connection"            return TOKEN(TCONNECTION_KW);

"autotuner"             return TOKEN(TAUTOTUNER);
"state"                 return TOKEN(TSTATE);
"params"                return TOKEN(TPARAMS);
"temp"                  return TOKEN(TTEMP);
"measurement"           return TOKEN(TMEASUREMENT);
"run"                   return TOKEN(TRUN);
"start"                 return TOKEN(TSTART);
"requires"              return TOKEN(TREQUIRES);
"terminal"              return TOKEN(TTERMINAL);
"if"                    return TOKEN(TIF);
"else"                  return TOKEN(TELSE);
"true"                  return TOKEN(TTRUE);
"false"                 return TOKEN(TFALSE);
"success"               return TOKEN(TSUCCESS);
"fail"                  return TOKEN(TFAIL);
"spec-inputs"           return TOKEN(TSPEC_INPUTS);
"spec-outputs"          return TOKEN(TSPEC_OUTPUTS);
"config"                return TOKEN(TCONFIG_VAR);
"next"                  return TOKEN(TNEXT);
"for"                   return TOKEN(TFOR);
"in"                    return TOKEN(TIN);

"."                     return TOKEN(TDOT);
"->"                    return TOKEN(TARROW);
"::"                    return TOKEN(TDOUBLECOLON);
"["                     return TOKEN(TLBRACKET);
"]"                     return TOKEN(TRBRACKET);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"="                     return TOKEN(TASSIGN);
","                     return TOKEN(TCOMMA);
":"                     return TOKEN(TCOLON);
";"                     return TOKEN(TSEMICOLON);

"+"                     return TOKEN(TPLUS);
"-"                     return TOKEN(TMINUS);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
"=="                    return TOKEN(TEQ);
"!="                    return TOKEN(TNE);
"<"                     return TOKEN(TLL);
">"                     return TOKEN(TGG);
"<="                    return TOKEN(TLE);
">="                    return TOKEN(TGE);
"&&"                    return TOKEN(TAND);
"||"                    return TOKEN(TOR);
"!"                     return TOKEN(TNOT);

[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TIDENTIFIER;
[0-9]+\.[0-9]*([eE][-+]?[0-9]+)? SAVE_TOKEN; return TDOUBLE;
[0-9]+[eE][-+]?[0-9]+            SAVE_TOKEN; return TDOUBLE;
[0-9]+                           SAVE_TOKEN; return TINTEGER;
\"(\\.|[^"\\])*\"                SAVE_TOKEN; return TSTRING;

.                       printf("Unknown token! %s\n", yytext); yyterminate();

%%
