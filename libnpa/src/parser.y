%{
#ifdef __ANDROID__
#include <android/log.h>
#endif
    #include "nsscompiler.hpp"
    #include <cstdio>
    #include <cstdlib>
    #include <boost/algorithm/string/replace.hpp>

    Call* MakeCall(string Name, uint16_t Magic);
    void WriteSymbol(const string& Symbol);
    Block* WrapStmt(Statement* s) { Block* b = new Block(); b->Statements.push_back(s); return b; }

    Program* pRoot;
    extern int yylex();
    void yyerror(const char* s) { extern int g_nss_line; extern char* yytext; extern int yyleng;
    std::fprintf(stderr, "Error: %s at line %d near '%s'\n", s, g_nss_line, yytext);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_ERROR, "muramasa", "nss parse error: %s line %d near '%s'", s, g_nss_line, yytext);
#endif
    std::abort(); }
%}

%union
{
    Node* node;
    Program* program;
    Block* block;
    Statement* stmt;
    Argument* arg;
    Expression* expr;
    Subroutine* subr;
    std::vector<Argument*>* argvec;
    std::vector<Expression*>* expvec;
    std::string* string;
    int token;
}

%token <string> TIDENTIFIER TFLOAT TINTEGER TXML TSTRING THEX TVARNAME
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TFUNCTION TSEMICOLON TEQUAL TCOMMA TQUOTE TCHAPTER TSCENE TINCLUDE
%token <token> TADD TSUB TMUL TDIV TMOD TIF TWHILE TLESS TGREATER TEQUALEQUAL TNEQUAL TGEQUAL TLEQUAL TAND TOR TNOT
%token <token> TRETURN TCALLCHAPTER TCALLSCENE TELSE TSELECT TCASE TCOLON TAT TDOLLAR THASH TBREAK TLABRACE TRABRACE TARROW

%type <program> start
%type <arg> arg array
%type <argvec> func_args
%type <expvec> func_exps
%type <block> stmts block
%type <stmt> stmt call cond
%type <subr> func_decl
%type <expr> expr

%left TOR
%left TAND
%nonassoc TEQUALEQUAL TNEQUAL
%nonassoc TLESS TGREATER TGEQUAL TLEQUAL
%left TADD TSUB
%left TMUL TDIV TMOD
%left UNARY

%start start

%%

start : { pRoot = new Program; } program { }
      ;

program : func_decl { pRoot->Subroutines.push_back($1); }
        | program func_decl { pRoot->Subroutines.push_back($2); }
        | include
        | program include
        ;

include : TINCLUDE TSTRING { WriteSymbol("include." + $2->substr(1, $2->size() - 2)); delete $2; }
        ;

stmts : stmt { $$ = new Block(); $$->Statements.push_back($<stmt>1); }
      | stmts stmt { $1->Statements.push_back($<stmt>2); }
      ;

stmt : call | cond | block
     | arg TEQUAL expr TSEMICOLON { $$ = new Assignment($1, $3, MAGIC_ASSIGN); }
     | arg TADD TEQUAL expr TSEMICOLON { $$ = new Assignment($1, $4, MAGIC_ADD_ASSIGN); }
     | arg TSUB TEQUAL expr TSEMICOLON { $$ = new Assignment($1, $4, MAGIC_SUB_ASSIGN); }
     | arg TMOD TEQUAL expr TSEMICOLON { $$ = new Assignment($1, $4, MAGIC_MOD_ASSIGN); }
     | arg TADD TADD TSEMICOLON { $$ = new UnaryStatement(MAGIC_INCREMENT, $1); }
     | arg TSUB TSUB TSEMICOLON { $$ = new UnaryStatement(MAGIC_DECREMENT, $1); }
     | arg TADD TADD TCOLON { $$ = new UnaryStatement(MAGIC_INCREMENT, $1); }
     | arg TSUB TSUB TCOLON { $$ = new UnaryStatement(MAGIC_DECREMENT, $1); }
     | TIDENTIFIER TCOLON { $$ = new Label(*$1); delete $1; }
     | TSEMICOLON { $$ = new Block(*new StatementList()); }
     ;

block : TLBRACE stmts TRBRACE { $$ = $2; }
      | TLBRACE TRBRACE { $$ = new Block(); }
      ;

func_decl : TFUNCTION arg TLPAREN func_args TRPAREN block { $2->Data = string("function.") + $2->Data; $$ = new Function($2, *$4, $6); delete $4; }
          | TCHAPTER arg block { $2->Data = string("chapter.") + $2->Data; $$ = new Chapter($2, $3); }
          | TSCENE arg block { $2->Data = string("scene.") + $2->Data; $$ = new Scene($2, $3); }
          ;

func_args : { $$ = new ArgumentList(); }
          | arg { $$ = new ArgumentList(); $$->push_back($<arg>1); }
          | func_args TCOMMA arg { $1->push_back($<arg>3); }
          ;

func_exps : { $$ = new ExpressionList(); }
          | expr { $$ = new ExpressionList(); $$->push_back($<expr>1); }
          | func_exps TCOMMA expr { $1->push_back($<expr>3); }
          | func_exps expr { $1->push_back($<expr>2); }
          ;

arg : TDOLLAR TIDENTIFIER { $$ = new Argument('$' + *$2, ARG_VARIABLE); delete $2; }
      | THASH TIDENTIFIER { $$ = new Argument('#' + *$2, ARG_VARIABLE); delete $2; }
      | THEX { $$ = new Argument(*$1, ARG_STRING); delete $1; }
      | TIDENTIFIER { $$ = new Argument(*$1, ARG_STRING); delete $1; }
      | TSTRING { std::string str = $1->substr(1, $1->size() - 2); boost::replace_all(str, "\\n", "\n"); $$ = new Argument(str, ARG_STRING); delete $1; }
      | TINTEGER { $$ = new Argument(*$1, ARG_INT); delete $1; }
      | TFLOAT { $$ = new Argument(*$1, ARG_FLOAT); delete $1; }
      | array { $$ = $1; }
      | THASH { $$ = new Argument("#", ARG_STRING); }
      | TDOLLAR { $$ = new Argument("$", ARG_STRING); }
      | TVARNAME { $$ = new Argument(*$1, ARG_VARIABLE); delete $1; }
      | TDOLLAR TDOLLAR TIDENTIFIER { $$ = new Argument('$' + *$3, ARG_VARIABLE); delete $3; }
      | TAT { $$ = new Argument("@", ARG_STRING); }
      ;

array : TDOLLAR TIDENTIFIER TLABRACE arg TRABRACE { $$ = new Array('$' + *$2); ((Array*)$$)->Arguments.push_back($4); delete $2; }
      | THASH TIDENTIFIER TLABRACE arg TRABRACE { $$ = new Array('#' + *$2); ((Array*)$$)->Arguments.push_back($4); delete $2; }
      | array TLABRACE arg TRABRACE { $$ = $1; ((Array*)$$)->Arguments.push_back($3); }
      | TVARNAME TLABRACE arg TRABRACE { $$ = new Array(*$1); ((Array*)$$)->Arguments.push_back($3); delete $1; }

call : arg TLPAREN func_exps TRPAREN TSEMICOLON { $$ = new CallStatement($1, *$3, MAGIC_CALL_FUNCTION); delete $3; }
     | TRETURN TSEMICOLON
          {
               ExpressionList Args;
               Argument* Arg = new Argument("Return", ARG_FUNCTION);
               $$ = new Call(Arg, Args, 0);
          }
     | TBREAK TSEMICOLON
          {
               ExpressionList Args;
               Argument* Arg = new Argument("Break", ARG_FUNCTION);
               $$ = new Call(Arg, Args, 0);
          }
     | TXML {
               size_t i = $1->find("<PRE ") + 5;
               size_t j = $1->find(">", i);
               size_t k = $1->find("[", j + 1);
               size_t l = k == std::string::npos ? std::string::npos : $1->find("]", k + 1);
               ExpressionList Args;
               if (k != std::string::npos && l != std::string::npos)
                    Args.push_back(new Argument($1->substr(k + 1, l - k - 1), ARG_STRING));
               else
                    Args.push_back(new Argument("", ARG_STRING));
               Args.push_back(new Argument(j != std::string::npos ? $1->substr(i, j - i) : *$1, ARG_STRING));
               Args.push_back(new Argument(*$1, ARG_STRING));
               Argument* Arg = new Argument("ParseText", ARG_FUNCTION);
               $$ = new Call(Arg, Args, 0);
               delete $1;
            }
     | TCALLCHAPTER TIDENTIFIER TSEMICOLON { $$ = MakeCall(*$2, MAGIC_CALL_CHAPTER); delete $2; }
     | TCALLCHAPTER TDOLLAR TIDENTIFIER TSEMICOLON { $$ = MakeCall('$' + *$3, MAGIC_CALL_CHAPTER); delete $3; }
     | TCALLCHAPTER arg TARROW arg TSEMICOLON { $$ = MakeCall($2->Data + "->" + $4->Data, MAGIC_CALL_CHAPTER); }
     | TCALLCHAPTER TAT TARROW arg TSEMICOLON { $$ = MakeCall("@->" + $4->Data, MAGIC_CALL_CHAPTER); }
     | TCALLSCENE TIDENTIFIER TSEMICOLON { $$ = MakeCall(*$2, MAGIC_CALL_SCENE); delete $2; }
     | TCALLSCENE TDOLLAR TIDENTIFIER TSEMICOLON { $$ = MakeCall('$' + *$3, MAGIC_CALL_SCENE); delete $3; }
     | TCALLSCENE arg TARROW arg TSEMICOLON { $$ = MakeCall($2->Data + "->" + $4->Data, MAGIC_CALL_SCENE); }
     | TCALLSCENE TAT TARROW arg TSEMICOLON { $$ = MakeCall("@->" + $4->Data, MAGIC_CALL_SCENE); }
     ;

expr : arg { $<arg>$ = $1; }
     | expr TMUL expr { $$ = new BinaryOperator($1, MAGIC_MUL_EXPRESSION, $3); }
     | expr TDIV expr { $$ = new BinaryOperator($1, MAGIC_DIV_EXPRESSION, $3); }
     | expr TMOD expr { $$ = new BinaryOperator($1, MAGIC_MOD_EXPRESSION, $3); }
     | expr TADD expr { $$ = new BinaryOperator($1, MAGIC_ADD_EXPRESSION, $3); }
     | expr TSUB expr { $$ = new BinaryOperator($1, MAGIC_SUB_EXPRESSION, $3); }
     | expr TLESS expr { $$ = new BinaryOperator($1, MAGIC_CMP_LESS, $3); }
     | expr TGREATER expr { $$ = new BinaryOperator($1, MAGIC_CMP_GREATER, $3); }
     | expr TEQUALEQUAL expr { $$ = new BinaryOperator($1, MAGIC_CMP_EQUAL, $3); }
     | expr TNEQUAL expr { $$ = new BinaryOperator($1, MAGIC_CMP_NE, $3); }
     | expr TGEQUAL expr { $$ = new BinaryOperator($1, MAGIC_CMP_GE, $3); }
     | expr TLEQUAL expr { $$ = new BinaryOperator($1, MAGIC_CMP_LE, $3); }
     | expr TAND expr { $$ = new BinaryOperator($1, MAGIC_CMP_LOGICAL_AND, $3); }
     | expr TOR expr { $$ = new BinaryOperator($1, MAGIC_CMP_LOGICAL_OR, $3); }
     | TNOT expr %prec UNARY { $$ = new UnaryOperator(MAGIC_NOT_EXPRESSION, $2); }
     | TSUB expr %prec UNARY { $$ = new UnaryOperator(MAGIC_NEGA_EXPRESSION, $2); }
     | TAT expr %prec UNARY { $$ = new UnaryOperator(MAGIC_AT_EXPRESSION, $2); }
     | arg TLPAREN func_exps TRPAREN { $$ = new Call($1, *$3, MAGIC_CALL_FUNCTION); delete $3; }
     | TLPAREN expr TRPAREN { $$ = $2; }
     | TADD expr %prec UNARY { $$ = $2; }
     ;

cond : TIF TLPAREN expr TRPAREN block { $$ = new If($5, $3); }
     | TIF TLPAREN expr TRPAREN block TELSE cond { $$ = new If($5, $3, $7); }
     | TIF TLPAREN expr TRPAREN block TELSE block { $$ = new If($5, $3, $7); }
     | TIF TLPAREN expr TRPAREN call { $$ = new If(WrapStmt($5), $3); }
     | TIF TLPAREN expr TRPAREN block TELSE call { $$ = new If($5, $3, WrapStmt($7)); }
     | TIF TLPAREN expr TRPAREN call TELSE call { $$ = new If(WrapStmt($5), $3, WrapStmt($7)); }
     | TIF TLPAREN expr TRPAREN call TELSE cond { $$ = new If(WrapStmt($5), $3, $7); }
     | TIF TLPAREN expr TRPAREN call TELSE block { $$ = new If(WrapStmt($5), $3, $7); }
     | TSELECT block { $$ = new Select($2); }
     | TCASE TIDENTIFIER TCOLON block { $$ = new Case(*$2, $4); delete $2; }
     | TCASE TIDENTIFIER TDIV TIDENTIFIER block { $$ = new Case(*$2 + "/" + *$4, $5); delete $2; delete $4; }
     | TCASE TIDENTIFIER TDIV TIDENTIFIER TCOLON block { $$ = new Case(*$2 + "/" + *$4, $6); delete $2; delete $4; }
     | TCASE TIDENTIFIER block { $$ = new Case(*$2, $3); delete $2; }
     | TCASE TAT TIDENTIFIER TCOLON block { $$ = new Case("@" + *$3, $5); delete $3; }
     | TCASE TAT TIDENTIFIER block { $$ = new Case("@" + *$3, $4); delete $3; }
     | TWHILE TLPAREN expr TRPAREN block { $$ = new While($5, $3); }
     | TWHILE TLPAREN expr TRPAREN call { $$ = new While(WrapStmt($5), $3); }
     ;

%%
