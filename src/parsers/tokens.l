%{
#include <string>
#include "parsers/parser_inc.h"
#include "parser.hpp"


#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
%}

%option noyywrap
%option never-interactive
%option noinput
%option nounput

%%

[ \t\n]                 ;
"<"                     return TOKEN(LPAR);
">"                     return TOKEN(RPAR);
","                     return TOKEN(SEP);
"."                     return TOKEN(TDOT);
"net"                   return TOKEN(TNET);
"->"                    return TOKEN(TOUT);
"<-"                    return TOKEN(TIN);
"true"                  return TOKEN(TTRUE);
"false"                 return TOKEN(TFALSE);
[0-9]+                  SAVE_TOKEN; return TINTEGER;
[0-9]+.[0-9]*           SAVE_TOKEN; return TREAL;
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TNAME;
.                       printf("Unknown token!\n"); yyterminate();

%%

extern int yyparse(StatementUnion&);

StatementUnion parse_line(const std::string& in) {
	YY_BUFFER_STATE s = yy_scan_string(in.c_str());
    StatementUnion statement;
    yyparse(statement);
    yy_delete_buffer(s);
	return statement;
}