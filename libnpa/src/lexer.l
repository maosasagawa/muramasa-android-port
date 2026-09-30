%{
#include <string>
#include "nsscompiler.hpp"
#include "parser.hpp"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
int g_nss_line = 1;
std::string g_vtext;
std::string g_str;
extern void nss_count_lines(const char* s, int n);
%}

%option noyywrap
%x COMMENT
%x VTEXT
%x STRG
%x PRETEXT

%%

"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>\n { g_nss_line++; }
<COMMENT>. ;
"<PRE"[^>\n]*">"     { g_vtext.assign(yytext, yyleng); BEGIN(PRETEXT); }
"<voice"[^>\n]*">" { g_vtext.assign(yytext, yyleng); BEGIN(VTEXT); }
<PRETEXT>^"</PRE>"    { g_vtext += yytext; BEGIN(INITIAL); yylval.string = new std::string(g_vtext); return TXML; }
<PRETEXT>\n          { g_nss_line++; g_vtext += yytext; }
<PRETEXT>.           { g_vtext += yytext; }
<PRETEXT><<EOF>>     { BEGIN(INITIAL); yylval.string = new std::string(g_vtext); return TXML; }
"</PRE>"            ;
<VTEXT>^"</PRE>" { g_vtext += yytext; BEGIN(INITIAL); yylval.string = new std::string(g_vtext); return TXML; }
<VTEXT>\n { g_nss_line++; g_vtext += yytext; }
<VTEXT>. { g_vtext += yytext; }
<VTEXT><<EOF>> { BEGIN(INITIAL); yylval.string = new std::string(g_vtext); return TXML; }
^continuation[^>\n]*">"[^\n]* ;
[ \t\n\r]               { if(yytext[0]=='\n') g_nss_line++; }
"call_chapter"          return TOKEN(TCALLCHAPTER);
"call_scene"            return TOKEN(TCALLSCENE);
"function"              return TOKEN(TFUNCTION);
"chapter"               return TOKEN(TCHAPTER);
"scene"                 return TOKEN(TSCENE);
"if"                    return TOKEN(TIF);
"else"                  return TOKEN(TELSE);
"while"                 return TOKEN(TWHILE);
"select"                return TOKEN(TSELECT);
"case"                  return TOKEN(TCASE);
"return"                return TOKEN(TRETURN);
"break"                 return TOKEN(TBREAK);
"$"[A-Za-z_]+:[^$\n]*"$" ;
"+"[ \t]*"//"[^\n]* ;
\xE3\x80\x80 ;
[$#][0-9]+[/.a-zA-Z\x80-\xFF0-9_*]* {
                            const char* sl = strchr(yytext, '/');
                            if (sl && !strstr(yytext, ".nss"))
                                yyless(sl - yytext);
                            const char* st = strchr(yytext, '*');
                            if (st && st[1] != '\0')
                                yyless(st - yytext);           // '*' mid-token: always split
                            else if (st)
                            {
                                int nx = yyinput();
                                unput(nx);
                                if (!(nx == EOF || nx == ',' || nx == ')' || nx == ';' ||
                                      nx == '{' || nx == '}' || nx == ':' ||
                                      nx == ' ' || nx == '\t' || nx == '\n' || nx == '\r'))
                                    yyless(st - yytext);
                            }
                            SAVE_TOKEN; return TVARNAME;
                          }
[a-zA-Z_\x80-\xFF][/.a-zA-Z\x80-\xFF0-9_*]*  {
                            const char* sl = strchr(yytext, '/');
                            if (sl && !strstr(yytext, ".nss"))
                                yyless(sl - yytext);
                            const char* st = strchr(yytext, '*');
                            if (st && st[1] != '\0')
                                yyless(st - yytext);           // '*' mid-token: always split
                            else if (st)
                            {
                                int nx = yyinput();
                                unput(nx);
                                if (!(nx == EOF || nx == ',' || nx == ')' || nx == ';' ||
                                      nx == '{' || nx == '}' || nx == ':' ||
                                      nx == ' ' || nx == '\t' || nx == '\n' || nx == '\r'))
                                    yyless(st - yytext);
                            }
                            SAVE_TOKEN; return TIDENTIFIER;
                          }
[\"]                    { g_str.assign(yytext, yyleng); BEGIN(STRG); }
<STRG>\\.           { g_str += yytext; }
<STRG>[\"]             {
                          int c = yyinput();
                          if (c == EOF)
                          {
                              g_str += '"';
                              BEGIN(INITIAL);
                              yylval.string = new std::string(g_str);
                              return TSTRING;
                          }
                          unput(c);
                          if (c == ',' || c == ')' || c == ']' || c == ';' || c == '{' || c == '}' ||
                              c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ':' || c == '=' ||
                              c == '<' || c == '>' || c == '+' || c == '&' || c == '|' || c == '%' || c == '-')
                          {
                              g_str += '"';
                              BEGIN(INITIAL);
                              yylval.string = new std::string(g_str);
                              return TSTRING;
                          }
                          g_str += '"';
                        }
<STRG>\n               { g_str += yytext; g_nss_line++; }
<STRG>.                 { g_str += yytext; }
<STRG><<EOF>>           { g_str += '"'; BEGIN(INITIAL); yylval.string = new std::string(g_str); return TSTRING; }
[0-9]+\.[0-9]*          SAVE_TOKEN; return TFLOAT;
[0-9]+                  SAVE_TOKEN; return TINTEGER;
"#"[0-9A-Fa-f]{6}       SAVE_TOKEN; return THEX;
","                     return TOKEN(TCOMMA);
";"                     return TOKEN(TSEMICOLON);
":"                     return TOKEN(TCOLON);
"$"                     return TOKEN(TDOLLAR);
"#"                     return TOKEN(THASH);
"="                     return TOKEN(TEQUAL);
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);
"["                     return TOKEN(TLABRACE);
"]"                     return TOKEN(TRABRACE);
"+"                     return TOKEN(TADD);
"-"                     return TOKEN(TSUB);
"/"                     return TOKEN(TDIV);
"%"                     return TOKEN(TMOD);
"*"                     return TOKEN(TMUL);
"<"                     return TOKEN(TLESS);
">"                     return TOKEN(TGREATER);
"->"                    return TOKEN(TARROW);
"=="                    return TOKEN(TEQUALEQUAL);
"!="                    return TOKEN(TNEQUAL);
">="                    return TOKEN(TGEQUAL);
"<="                    return TOKEN(TLEQUAL);
"&&"                    return TOKEN(TAND);
"||"                    return TOKEN(TOR);
"!"                     return TOKEN(TNOT);
"@"                     return TOKEN(TAT);
"#include"              return TOKEN(TINCLUDE);
"BezierMove"[^\n]* { /* unsupported point-list builtin: replace with a short wait so enclosing loops yield */
                            unput(';'); unput(')'); unput('0'); unput('2');
                            unput('('); unput('t'); unput('i'); unput('a'); unput('W'); }
"//"[^\n]*
^"."+[^\n]* ;
"."                     return TOKEN(TCOMMA);
.                       printf("Unknown token %s at line %d!\n", yytext, g_nss_line); yyterminate();

%%

void nss_count_lines(const char* s, int n){ for(int i=0;i<n;i++) if(s[i]=='\n') g_nss_line++; }
