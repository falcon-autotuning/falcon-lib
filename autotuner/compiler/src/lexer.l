%{
#include "parser.tab.hpp"

#undef YY_DECL
#define YY_DECL falcon::atc::Parser::symbol_type yylex(falcon::atc::Parser::location_type *loc)

int yycolumn = 1;

#define YY_USER_ACTION \
  loc->begin.line = loc->end.line = yylineno; \
  loc->begin.column = yycolumn; \
  loc->end.column = yycolumn + yyleng - 1; \
  yycolumn += yyleng;

#undef yyterminate
#define yyterminate() return falcon::atc::Parser::make_YYEOF(*loc)

using namespace falcon::atc;
%}

%option noyywrap
%option yylineno

%%

"autotuner"     { return Parser::make_AUTOTUNER(*loc); }
"routine"       { return Parser::make_ROUTINE(*loc); }
"state"         { return Parser::make_STATE(*loc); }
"start"         { return Parser::make_START(*loc); }
"requires"      { return Parser::make_REQUIRES(*loc); }
"terminal"      { return Parser::make_TERMINAL(*loc); }
"if"            { return Parser::make_IF(*loc); }
"else"          { return Parser::make_ELSE(*loc); }
"true"          { return Parser::make_TRUE(*loc); }
"false"         { return Parser::make_FALSE(*loc); }
"nil"           { return Parser::make_NIL(*loc); }
"config"        { return Parser::make_CONFIG_VAR(*loc); }

"float"         { return Parser::make_FLOAT_KW(*loc); }
"int"           { return Parser::make_INT_KW(*loc); }
"bool"          { return Parser::make_BOOL_KW(*loc); }
"string"        { return Parser::make_STRING_KW(*loc); }
"Quantity"      { return Parser::make_QUANTITY_KW(*loc); }
"Config"        { return Parser::make_CONFIG_KW(*loc); }
"Connection"    { return Parser::make_CONNECTION_KW(*loc); }
"Connections"   { return Parser::make_CONNECTIONS_KW(*loc); }
"Gname"         { return Parser::make_GNAME_KW(*loc); }
"Error"         { return Parser::make_ERROR_KW(*loc); }

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

[ \t\r]+        { /* skip whitespace */ }
\n              { yycolumn = 1; }
"//"[^\n]*      { /* skip comments */ }

.               { std::cerr << "Unknown character at line " << yylineno 
                            << ", col " << yycolumn << ": " << yytext << std::endl; }

%%

namespace falcon::atc {
  static Parser::location_type current_location;
  
  Parser::symbol_type yylex() {
    return ::yylex(&current_location);
  }
}
