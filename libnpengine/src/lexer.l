%{
#include <string>
#include "TextParser.hpp"
#include "Parser.hpp"
#define SAVE_TOKEN xmllval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (xmllval.token = t)
%}

%option noyywrap
%option prefix="xml"

%%

"\n\n"                  return TOKEN(TNEWLINE);
[ \t\n\r]               ;
"PRE"                   return TOKEN(TPRE);
"RUBY"                  return TOKEN(TRUBY);
"FONT"                  return TOKEN(TFONT);
"voice"                 return TOKEN(TVOICE);
"#"[0-9A-Fa-f]{6}       SAVE_TOKEN; return THEX;
\{[^}]*\}               ;
"//"[^\n]*              ;
[?a-zA-Z_\x80-\xFF@][?,'/.a-zA-Z_\x80-\xFF０-９0-9@]*  SAVE_TOKEN; return TSTRING;
"<"                     return TOKEN(TLBRACE);
">"                     return TOKEN(TRBRACE);
"["                     return TOKEN(TLABRACE);
"]"                     return TOKEN(TRABRACE);
"\""                    return TOKEN(TQUOTE);
"="                     return TOKEN(TEQUAL);
"/"                     return TOKEN(TSLASH);
.                       printf("Unknown token!\n"); yyterminate();

%%
