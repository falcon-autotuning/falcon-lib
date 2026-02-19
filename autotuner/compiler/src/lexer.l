%{
#include "parser.tab.hpp"

// Don't try to define yylex in a namespace with YY_DECL
// Instead, define it in global scope and wrap it
#undef YY_DECL
#define YY_DECL falcon::atc::Parser::symbol_type yylex(falcon::atc::Parser::location_type *loc)

// Track column position manually - make it non-static so we can reset it externally
int yycolumn = 1;  // ← REMOVE 'static' keyword

// Update location on every token
#define YY_USER_ACTION \
  loc->begin.line = loc->end.line = yylineno; \
  loc->begin.column = yycolumn; \
  loc->end.column = yycolumn + yyleng - 1; \
  yycolumn += yyleng;

// Override yyterminate to return proper EOF token with location
#undef yyterminate
#define yyterminate() return falcon::atc::Parser::make_YYEOF(*loc)

// Using directive so we can use Parser::make_* without qualification
using namespace falcon::atc;
%}

%option noyywrap
%option yylineno

%%

"autotuner"     { return Parser::make_AUTOTUNER(*loc); }
"state"         { return Parser::make_STATE(*loc); }
"params"        { return Parser::make_PARAMS(*loc); }
"temp"          { return Parser::make_TEMP(*loc); }
"measurement"   { return Parser::make_MEASUREMENT(*loc); }
"run"           { return Parser::make_RUN(*loc); }
"start"         { return Parser::make_START(*loc); }
"requires"      { return Parser::make_REQUIRES(*loc); }
"terminal"      { return Parser::make_TERMINAL(*loc); }
"if"            { return Parser::make_IF(*loc); }
"else"          { return Parser::make_ELSE(*loc); }
"true"          { return Parser::make_TRUE(*loc); }
"false"         { return Parser::make_FALSE(*loc); }
"success"       { return Parser::make_SUCCESS(*loc); }
"fail"          { return Parser::make_FAIL(*loc); }
"spec_inputs"   { return Parser::make_SPEC_INPUTS(*loc); }
"spec_outputs"  { return Parser::make_SPEC_OUTPUTS(*loc); }
"config"        { return Parser::make_CONFIG_VAR(*loc); }
"next"          { return Parser::make_NEXT(*loc); }
"for"           { return Parser::make_FOR(*loc); }
"in"            { return Parser::make_IN(*loc); }

"float"         { return Parser::make_FLOAT_KW(*loc); }
"int"           { return Parser::make_INT_KW(*loc); }
"bool"          { return Parser::make_BOOL_KW(*loc); }
"string"        { return Parser::make_STRING_KW(*loc); }
"quantity"      { return Parser::make_QUANTITY_KW(*loc); }
"Config"        { return Parser::make_CONFIG_KW(*loc); }
"Group"         { return Parser::make_GROUP_KW(*loc); }
"Connection"    { return Parser::make_CONNECTION_KW(*loc); }

"->"            { return Parser::make_ARROW(*loc); }
"::"            { return Parser::make_DOUBLECOLON(*loc); }
"["             { return Parser::make_LBRACKET(*loc); }
"]"             { return Parser::make_RBRACKET(*loc); }
"{"             { return Parser::make_LBRACE(*loc); }
"}"             { return Parser::make_RBRACE(*loc); }
"("             { return Parser::make_LPAREN(*loc); }
")"             { return Parser::make_RPAREN(*loc); }
"="             { return Parser::make_ASSIGN(*loc); }
","             { return Parser::make_COMMA(*loc); }
":"             { return Parser::make_COLON(*loc); }
";"             { return Parser::make_SEMICOLON(*loc); }
"."             { return Parser::make_DOT(*loc); }
"+"             { return Parser::make_PLUS(*loc); }
"-"             { return Parser::make_MINUS(*loc); }
"*"             { return Parser::make_MUL(*loc); }
"/"             { return Parser::make_DIV(*loc); }
"=="            { return Parser::make_EQ(*loc); }
"!="            { return Parser::make_NE(*loc); }
"<"             { return Parser::make_LL(*loc); }
">"             { return Parser::make_GG(*loc); }
"<="            { return Parser::make_LE(*loc); }
">="            { return Parser::make_GE(*loc); }
"&&"            { return Parser::make_AND(*loc); }
"||"            { return Parser::make_OR(*loc); }
"!"             { return Parser::make_NOT(*loc); }

[0-9]+          { return Parser::make_INTEGER(yytext, *loc); }
[0-9]+\.[0-9]+  { return Parser::make_DOUBLE(yytext, *loc); }
\"[^\"]*\"      { return Parser::make_STRING(std::string(yytext+1, yyleng-2), *loc); }
[a-zA-Z_][a-zA-Z0-9_]* { return Parser::make_IDENTIFIER(yytext, *loc); }

[ \t\r]+        { /* skip whitespace (not newlines) */ }
\n              { yycolumn = 1; /* reset column on newline, then continue */ }
"//"[^\n]*      { /* skip comments */ }

.               { std::cerr << "Unknown character at line " << yylineno 
                            << ", col " << yycolumn << ": " << yytext << std::endl; }

%%

// NOW wrap the global yylex() in the namespace the parser expects
namespace falcon::atc {
  // Store location for the wrapper to pass
  static Parser::location_type current_location;
  
  Parser::symbol_type yylex() {
    return ::yylex(&current_location);  // Pass location to the global yylex
  }
}
