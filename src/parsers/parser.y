%{
#include "parsers/parser_inc.h"

extern int yylex();
static void yyerror(StatementUnion& u, const char *s) { u = std::string(s); }

%}

%union {
    int64_t                                  int_constant;
    double                                   real_constant;
    bool                                     bool_constant;
    std::string*                             string;
	std::vector<std::string>*                args;
    cpplink::translator::ModuleDeclaration*  module_dec;
    cpplink::translator::NetPinCommand*      net_pin;
    cpplink::translator::NetConstCommand*    net_const;
	cpplink::translator::IoPinDeclaration*   io_pin;
	cpplink::translator::BlackboxCommand*    blackbox;
	cpplink::translator::GenericDeclaration* generic;
    int                                      token;
}

%type <int_constant>   int_constant
%type <real_constant>  real_constant
%type <bool_constant>  bool_constant   
%type <module_dec>     declaration
%type <net_pin>        net_pin
%type <net_const>      net_const
%type <token>          dir
%type <args>           arguments
%type <generic>        generic
%type <io_pin>         io_pin
%type <blackbox>       blackbox

%start statement

%parse-param {StatementUnion& root}

%error-verbose

%token <token>  TTRUE    "'true' constant"
%token <token>  TFALSE   "'false' constant"
%token <string> TINTEGER "integral constant"
%token <string> TREAL    "real constant"
%token <token>  LPAR     "left template parentheses"
%token <token>  RPAR     "right template parentheses"
%token <token>  SEP      ","
%token <token>  TDOT     "."
%token <token>  TNET     "keyword 'net'"
%token <token>  TOUT     "->"
%token <token>  TIN      "<-"
%token <string> TNAME    "name"
%token <token>  TUSING   "keyword 'using'"
%token <token>  TGENERIC "keyword 'generic'"
%token <token>  TBLACKBOX "keyword 'blackbox'"
%token <token>  TIN_WORD  "keyword 'modulein'"
%token <token>  TOUT_WORD "keyword 'moduleout'"


%%

statement : declaration { root = StatementUnion(*$1); delete $1; }
          | net_pin     { root = StatementUnion(*$1); delete $1; }
          | net_const   { root = StatementUnion(*$1); delete $1; }
		  | generic     { root = StatementUnion(*$1); delete $1; }
		  | io_pin      { root = StatementUnion(*$1); delete $1; }
		  | blackbox    { root = StatementUnion(*$1); delete $1; }
          ;

declaration : TNAME TNAME { $$ = new cpplink::translator::ModuleDeclaration{*$1, *$2, {}}; delete $1; delete $2; }
            | TNAME LPAR arguments TNAME { $$ = new cpplink::translator::ModuleDeclaration{*$1, *$4, {$3->rbegin(), $3->rend()}}; delete $1; delete $3; delete $4; }
            ;

arguments : TNAME SEP arguments { $$ = $3; $$->push_back(*$1); delete $1; }
		  | TNAME RPAR { $$ = new std::vector<std::string>({ *$1 }); delete $1; }
		  ;

net_pin : TNET TNAME TDOT TNAME dir TNAME { $$ = new cpplink::translator::NetPinCommand{*$6, *$2, *$4, $5 == TOUT}; delete $2; delete $4; delete $6; }
        ;

net_const : TNET int_constant TOUT TNAME  { $$ = new cpplink::translator::NetConstCommand{*$4, {$2}}; delete $4; }
          | TNET real_constant TOUT TNAME { $$ = new cpplink::translator::NetConstCommand{*$4, {$2}}; delete $4; }
          | TNET bool_constant TOUT TNAME { $$ = new cpplink::translator::NetConstCommand{*$4, {$2}}; delete $4; }
          ;

generic : TGENERIC TNAME { $$ = new cpplink::translator::GenericDeclaration{*$2}; delete $2; }
        ;

io_pin  : TIN_WORD TNAME TDOT TNAME  { $$ = new cpplink::translator::IoPinDeclaration{*$2, *$4, false}; delete $2; delete $4; }
        | TOUT_WORD TNAME TDOT TNAME { $$ = new cpplink::translator::IoPinDeclaration{*$2, *$4, true}; delete $2; delete $4; }
		;

blackbox : TBLACKBOX int_constant { $$ = new cpplink::translator::BlackboxCommand{static_cast<int32_t>($2)}; }
         ;

dir : TIN
    | TOUT
    ;

int_constant : TINTEGER { $$ = atol($1->c_str()); delete $1; }
             ;

real_constant : TREAL { $$ = atof($1->c_str()); delete $1; }
              ;

bool_constant : TTRUE { $$ = true; }
              | TFALSE { $$ = false; }
              ;

%%
