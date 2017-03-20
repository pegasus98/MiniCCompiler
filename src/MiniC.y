%{

#include <cstdio>
#include "yaccTypes.h"

extern int yylex (void);
void yyerror(const char *s);

%}

%union {
    char *vstr;
    char vchar;
    int vint;
    double vdouble;
}

%token <vstr> IDENTIFIER
%token <vstr> STR_CONSTANT
%token <vchar> CHAR_CONSTANT
%token <vint> INT_CONSTANT
%token <vint> ENUM_CONSTANT
%token <vdouble> DOUBLE_CONSTANT
%token SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
%token <vstr> TYPE_NAME
%token TYPEDEF
%token STATIC
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOID
%token STRUCT UNION ENUM
%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%nonassoc IFX
%nonassoc ELSE

%start translation_unit

%%
primary_expression:
	  IDENTIFIER
	| constant
	| string
	| '(' expression ')'
	;

constant:
	  INT_CONSTANT
    | CHAR_CONSTANT
	| DOUBLE_CONSTANT
	| ENUM_CONSTANT
	;

enumeration_constant:
	  IDENTIFIER
	;

string:
	  STR_CONSTANT
	;

postfix_expression:
	  primary_expression
	| postfix_expression '[' expression ']'
	| postfix_expression '(' ')'
	| postfix_expression '(' argument_expression_list ')'
	| postfix_expression '.' IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
	| '(' type_name ')' '{' initializer_list '}'
	| '(' type_name ')' '{' initializer_list ',' '}'
	;

argument_expression_list:
	  assignment_expression
	| argument_expression_list ',' assignment_expression
	;

unary_expression:
	  postfix_expression
	| INC_OP unary_expression
	| DEC_OP unary_expression
	| unary_operator cast_expression
	| SIZEOF unary_expression
	| SIZEOF '(' type_name ')'
	;

unary_operator:
	  '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

cast_expression:
	  unary_expression
	| '(' type_name ')' cast_expression
	;

multiplicative_expression:
	  cast_expression
	| multiplicative_expression '*' cast_expression
	| multiplicative_expression '/' cast_expression
	| multiplicative_expression '%' cast_expression
	;

additive_expression:
	  multiplicative_expression
	| additive_expression '+' multiplicative_expression
	| additive_expression '-' multiplicative_expression
	;

shift_expression:
	  additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

relational_expression:
	  shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	| relational_expression LE_OP shift_expression
	| relational_expression GE_OP shift_expression
	;

equality_expression:
	  relational_expression
	| equality_expression EQ_OP relational_expression
	| equality_expression NE_OP relational_expression
	;

and_expression:
	  equality_expression
	| and_expression '&' equality_expression
	;

exclusive_or_expression:
	  and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression:
	  exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression:
	  inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	;

logical_or_expression:
	  logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	;

conditional_expression:
	  logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression:
	  conditional_expression
	| unary_expression assignment_operator assignment_expression
	;

assignment_operator:
	  '='
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

expression:
	  assignment_expression
	| expression ',' assignment_expression
	;

constant_expression:
	  conditional_expression
	;

declaration:
	  declaration_specifiers ';'
	| declaration_specifiers init_declarator_list ';'
	;

declaration_specifiers:
	  storage_class_specifier declaration_specifiers
	| storage_class_specifier
	| type_specifier declaration_specifiers
	| type_specifier
	| type_qualifier declaration_specifiers
	| type_qualifier
	;

init_declarator_list:
	  init_declarator
	| init_declarator_list ',' init_declarator
	;

init_declarator:
	  declarator '=' initializer
	| declarator
	;

storage_class_specifier:
	  TYPEDEF
	| STATIC
	;

type_specifier:
	  VOID
	| CHAR
	| SHORT
	| INT
	| LONG
	| FLOAT
	| DOUBLE
	| SIGNED
	| UNSIGNED
	| struct_or_union_specifier
	| enum_specifier
	| TYPE_NAME
	;

struct_or_union_specifier:
	  struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER
	;

struct_or_union:
	  STRUCT
	| UNION
	;

struct_declaration_list:
	  struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration:
	  specifier_qualifier_list ';'
	| specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list:
	  type_specifier specifier_qualifier_list
	| type_specifier
	| type_qualifier specifier_qualifier_list
	| type_qualifier
	;

struct_declarator_list:
	  struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator:
	  ':' constant_expression
	| declarator ':' constant_expression
	| declarator
	;

enum_specifier:
	  ENUM '{' enumerator_list '}'
	| ENUM '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER
	;

enumerator_list:
	  enumerator
	| enumerator_list ',' enumerator
	;

enumerator:
	  enumeration_constant '=' constant_expression
	| enumeration_constant
	;

type_qualifier:
	  CONST
	;

declarator:
	  pointer direct_declarator
	| direct_declarator
	;

direct_declarator:
	  IDENTIFIER
	| '(' declarator ')'
	| direct_declarator '[' ']'
	| direct_declarator '[' '*' ']'
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
	| direct_declarator '[' STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list '*' ']'
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list assignment_expression ']'
	| direct_declarator '[' type_qualifier_list ']'
	| direct_declarator '[' assignment_expression ']'
	| direct_declarator '(' parameter_list ')'
	| direct_declarator '(' ')'
	| direct_declarator '(' identifier_list ')'
	;

pointer:
	  '*' type_qualifier_list pointer
	| '*' type_qualifier_list
	| '*' pointer
	| '*'
	;

type_qualifier_list:
	  type_qualifier
	| type_qualifier_list type_qualifier
	;


parameter_list:
	  parameter_declaration
	| parameter_list ',' parameter_declaration
	;

parameter_declaration:
	  declaration_specifiers declarator
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;

identifier_list:
	  IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name:
	  specifier_qualifier_list abstract_declarator
	| specifier_qualifier_list
	;

abstract_declarator:
	  pointer direct_abstract_declarator
	| pointer
	| direct_abstract_declarator
	;

direct_abstract_declarator:
	  '(' abstract_declarator ')'
	| '[' ']'
	| '[' '*' ']'
	| '[' STATIC type_qualifier_list assignment_expression ']'
	| '[' STATIC assignment_expression ']'
	| '[' type_qualifier_list STATIC assignment_expression ']'
	| '[' type_qualifier_list assignment_expression ']'
	| '[' type_qualifier_list ']'
	| '[' assignment_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' '*' ']'
	| direct_abstract_declarator '[' STATIC type_qualifier_list assignment_expression ']'
	| direct_abstract_declarator '[' STATIC assignment_expression ']'
	| direct_abstract_declarator '[' type_qualifier_list assignment_expression ']'
	| direct_abstract_declarator '[' type_qualifier_list STATIC assignment_expression ']'
	| direct_abstract_declarator '[' type_qualifier_list ']'
	| direct_abstract_declarator '[' assignment_expression ']'
	| '(' ')'
	| '(' parameter_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_list ')'
	;

initializer:
	  '{' initializer_list '}'
	| '{' initializer_list ',' '}'
	| assignment_expression
	;

initializer_list:
	  designation initializer
	| initializer
	| initializer_list ',' designation initializer
	| initializer_list ',' initializer
	;

designation:
	  designator_list '='
	;

designator_list:
	  designator
	| designator_list designator
	;

designator:
	  '[' constant_expression ']'
	| '.' IDENTIFIER
	;

statement:
	  labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement:
	  IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement:
	  '{' '}'
	| '{'  block_item_list '}'
	;

block_item_list:
	  block_item
	| block_item_list block_item
	;

block_item:
	  declaration
	| statement
	;

expression_statement:
	  ';'
	| expression ';'
	;

selection_statement:
	  IF '(' expression ')' statement ELSE statement
	| IF '(' expression ')' statement %prec IFX
	| SWITCH '(' expression ')' statement
	;

iteration_statement:
	  WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	| FOR '(' declaration expression_statement ')' statement
	| FOR '(' declaration expression_statement expression ')' statement
	;

jump_statement:
	  GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN expression ';'
	;

translation_unit:
	  external_declaration
	| translation_unit external_declaration
	;

external_declaration:
	  function_definition
	| declaration
	;

function_definition:
	  declaration_specifiers declarator declaration_list compound_statement
	| declaration_specifiers declarator compound_statement
	;

declaration_list:
	  declaration
	| declaration_list declaration
	;

%%

void yyerror(const char *s)
{
    puts(s);
}
