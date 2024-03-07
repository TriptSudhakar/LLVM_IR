%{
#include <cstdio>
#include <iostream>

#include "ASTNode.hpp"

using namespace std;

// stuff from flex that bison needs to know about:
extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;
 
void yyerror(const char *s);

ASTNode* root = NULL;


%}


%union{
	ASTNode* ast_node;
	char* str;
}

// %parse-param {ASTNode **root}

%token <str> IDENTIFIER I_CONSTANT
%token F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN
%token TYPEDEF_NAME ENUMERATION_CONSTANT

%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token CONST RESTRICT VOLATILE
%token BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token COMPLEX IMAGINARY 
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%type <ast_node> translation_unit external_declaration function_definition init_declarator_list static_assert_declaration
%type <ast_node> declaration_specifiers  declaration_list compound_statement storage_class_specifier type_specifier atomic_type_specifier struct_or_union_specifier enum_specifier direct_declarator pointer declarator type_qualifier_list parameter_type_list identifier_list 
%type <ast_node> block_item block_item_list declaration statement type_qualifier function_specifier alignment_specifier
%type <ast_node> labeled_statement expression_statement selection_statement iteration_statement jump_statement
%type <ast_node> expression assignment_expression assignment_operator
%type <ast_node> conditional_expression logical_or_expression logical_and_expression 
%type <ast_node> inclusive_or_expression exclusive_or_expression and_expression equality_expression relational_expression shift_expression additive_expression multiplicative_expression
%type <ast_node> unary_expression cast_expression type_name postfix_expression primary_expression initializer_list unary_operator argument_expression_list
%type <ast_node> generic_selection constant string

%start translation_unit

%%


primary_expression
	: IDENTIFIER					{ $$ = new ASTNode(Identifier); $$->m_value = yylval.str; }
	| constant						{ $$ = $1; }
	| string						{ $$ = new ASTNode(String); $$->pushChild($1); }
	| '(' expression ')'			{ $$ = $2; }
	| generic_selection				{ $$ = $1; }
	;

constant
	: I_CONSTANT					{ $$ = new ASTNode(I_Constant); $$->m_value = yylval.str; } /* includes character_constant */
	| F_CONSTANT					{ $$ = new ASTNode(Constant); $$->m_value = "F_CONSTANT"; }     // s. do this later
	| ENUMERATION_CONSTANT			{ $$ = new ASTNode(Constant); $$->m_value = "ENUMERATION_CONSTANT";  } /* after it has been defined as such */
	;

enumeration_constant		/* before it has been defined as such */
	: IDENTIFIER
	;

string
	: STRING_LITERAL
	| FUNC_NAME
	;

generic_selection
	: GENERIC '(' assignment_expression ',' generic_assoc_list ')'
	;

generic_assoc_list
	: generic_association
	| generic_assoc_list ',' generic_association
	;

generic_association
	: type_name ':' assignment_expression
	| DEFAULT ':' assignment_expression
	;

postfix_expression
	: primary_expression												{ $$ = new ASTNode(Primary_Expression); $$->pushChild($1); }
	| postfix_expression '[' expression ']'								{ $$ = new ASTNode(Postfix_Expression); $$->pushChild($1); $$->pushChild($3); }							
	| postfix_expression '(' ')'										{ $$ = new ASTNode(Postfix_Expression); $$->pushChild($1); $$->m_value = "()"; }
	| postfix_expression '(' argument_expression_list ')'				{ $$ = new ASTNode(Postfix_Expression); $$->pushChild($1); $$->pushChild($3); }
	| postfix_expression '.' IDENTIFIER									{ $$ = new ASTNode(Postfix_Expression); $$->pushChild($1); $$->m_value = ". IDENTIFIER"; }
	| postfix_expression PTR_OP IDENTIFIER								{ $$ = new ASTNode(Postfix_Expression); $$->pushChild($1); $$->m_value = "PTR_OP IDENTIFIER"; }
	| postfix_expression INC_OP											{ $$ = new ASTNode(Postfix_Expression); $$->pushChild($1); $$->m_value = "INC_OP"; }
	| postfix_expression DEC_OP											{ $$ = new ASTNode(Postfix_Expression); $$->pushChild($1); $$->m_value = "DEC_OP"; }
	| '(' type_name ')' '{' initializer_list '}'						{ $$ = new ASTNode(Postfix_Expression); $$->pushChild($2); $$->pushChild($5); }
	| '(' type_name ')' '{' initializer_list ',' '}'					{ $$ = new ASTNode(Postfix_Expression); $$->pushChild($2); $$->pushChild($5); }
	;

argument_expression_list
	: assignment_expression												{ $$ = new ASTNode(Argument_Expression_List); $$->pushChild($1); }
	| argument_expression_list ',' assignment_expression				{ $$ = new ASTNode(Argument_Expression_List); $$->pushChild($1); $$->pushChild($3); }
	;

unary_expression
	: postfix_expression				{ $$ = new ASTNode(Unary_Expression); $$->pushChild($1); }
	| INC_OP unary_expression			{ $$ = new ASTNode(Unary_Expression); $$->pushChild($2); $$->m_value = "INC_OP"; }
	| DEC_OP unary_expression			{ $$ = new ASTNode(Unary_Expression); $$->pushChild($2); $$->m_value = "DEC_OP"; }
	| unary_operator cast_expression	{ $$ = new ASTNode(Unary_Expression); $$->pushChild($1); $$->pushChild($2); }
	| SIZEOF unary_expression			{ $$ = new ASTNode(Unary_Expression); $$->pushChild($2); $$->m_value = "SIZEOF"; }
	| SIZEOF '(' type_name ')'			{ $$ = new ASTNode(Unary_Expression); $$->pushChild($3); $$->m_value = "SIZEOF"; }
	| ALIGNOF '(' type_name ')'			{ $$ = new ASTNode(Unary_Expression); $$->pushChild($3); $$->m_value = "ALIGNOF"; }
	;

unary_operator
	: '&'								{ $$ = new ASTNode(Unary_Operator); $$->m_value = "&"; }
	| '*'								{ $$ = new ASTNode(Unary_Operator); $$->m_value = "*"; }
	| '+'								{ $$ = new ASTNode(Unary_Operator); $$->m_value = "+"; }
	| '-'								{ $$ = new ASTNode(Unary_Operator); $$->m_value = "-"; }
	| '~'								{ $$ = new ASTNode(Unary_Operator); $$->m_value = "~"; }
	| '!'								{ $$ = new ASTNode(Unary_Operator); $$->m_value = "!"; }
	;

cast_expression
	: unary_expression															{ $$ = new ASTNode(Unary_Expression); $$->pushChild($1); }
	| '(' type_name ')' cast_expression											{ $$ = new ASTNode(Cast_Expression); $$->pushChild($2); $$->pushChild($4); }
	;

multiplicative_expression
	: cast_expression															{ $$ = new ASTNode(Cast_Expression); $$->pushChild($1); }
	| multiplicative_expression '*' cast_expression								{ $$ = new ASTNode(Multiplicative_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = "*"; }
	| multiplicative_expression '/' cast_expression								{ $$ = new ASTNode(Multiplicative_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = "/"; }
	| multiplicative_expression '%' cast_expression								{ $$ = new ASTNode(Multiplicative_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = "%"; }
	;

additive_expression
	: multiplicative_expression													{ $$ = new ASTNode(Multiplicative_Expression); $$->pushChild($1); }
	| additive_expression '+' multiplicative_expression							{ $$ = new ASTNode(Additive_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = "+"; }
	| additive_expression '-' multiplicative_expression							{ $$ = new ASTNode(Additive_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = "-"; }
	;

shift_expression
	: additive_expression														{ $$ = new ASTNode(Additive_Expression); $$->pushChild($1); }
	| shift_expression LEFT_OP additive_expression								{ $$ = new ASTNode(Shift_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = "<<"; }
	| shift_expression RIGHT_OP additive_expression								{ $$ = new ASTNode(Shift_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = ">>"; }
	;

relational_expression
	: shift_expression															{ $$ = new ASTNode(Shift_Expression); $$->pushChild($1); }
	| relational_expression '<' shift_expression								{ $$ = new ASTNode(Relational_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = "<"; }
	| relational_expression '>' shift_expression								{ $$ = new ASTNode(Relational_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = ">"; }
	| relational_expression LE_OP shift_expression								{ $$ = new ASTNode(Relational_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = "<="; }
	| relational_expression GE_OP shift_expression								{ $$ = new ASTNode(Relational_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = ">="; }
	;

equality_expression
	: relational_expression														{ $$ = new ASTNode(Relational_Expression); $$->pushChild($1); }
	| equality_expression EQ_OP relational_expression							{ $$ = new ASTNode(Equality_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = "=="; }
	| equality_expression NE_OP relational_expression							{ $$ = new ASTNode(Equality_Expression); $$->pushChild($1); $$->pushChild($3); $$->m_value = "!="; }
	;

and_expression
	: equality_expression														{ $$ = new ASTNode(Equality_Expression); $$->pushChild($1); }
	| and_expression '&' equality_expression									{ $$ = new ASTNode(And_Expression); $$->pushChild($1); $$->pushChild($3); }
	;

exclusive_or_expression
	: and_expression															{ $$ = new ASTNode(And_Expression); $$->pushChild($1); }
	| exclusive_or_expression '^' and_expression								{ $$ = new ASTNode(Exclusive_Or_Expression); $$->pushChild($1); $$->pushChild($3); }
	;

inclusive_or_expression
	: exclusive_or_expression													{ $$ = new ASTNode(Exclusive_Or_Expression); $$->pushChild($1); }
	| inclusive_or_expression '|' exclusive_or_expression						{ $$ = new ASTNode(Inclusive_Or_Expression); $$->pushChild($1); $$->pushChild($3); }
	;

logical_and_expression
	: inclusive_or_expression													{ $$ = new ASTNode(Inclusive_Or_Expression); $$->pushChild($1); }
	| logical_and_expression AND_OP inclusive_or_expression						{ $$ = new ASTNode(Logical_And_Expression); $$->pushChild($1); $$->pushChild($3); }
	;

logical_or_expression
	: logical_and_expression													{ $$ = new ASTNode(Logical_And_Expression); $$->pushChild($1); }
	| logical_or_expression OR_OP logical_and_expression						{ $$ = new ASTNode(Logical_Or_Expression); $$->pushChild($1); $$->pushChild($3); }
	;

conditional_expression
	: logical_or_expression														{ $$ = new ASTNode(Logical_Or_Expression); $$->pushChild($1); }
	| logical_or_expression '?' expression ':' conditional_expression			{ $$ = new ASTNode(Conditional_Expression); $$->pushChild($1); $$->pushChild($3); $$->pushChild($5); }
	;

assignment_expression
	: conditional_expression													{ $$ = new ASTNode(Conditional_Expression); $$->pushChild($1); }
	| unary_expression assignment_operator assignment_expression				{ $$ = new ASTNode(Assignment_Expression); $$->pushChild($1); $$->pushChild($2); $$->pushChild($3); }
	;

assignment_operator
	: '='					{ $$ = new ASTNode(Assignment_Operator); $$->m_value = "="; }
	| MUL_ASSIGN			{ $$ = new ASTNode(Assignment_Operator); $$->m_value = "*="; }
	| DIV_ASSIGN			{ $$ = new ASTNode(Assignment_Operator); $$->m_value = "/="; }
	| MOD_ASSIGN			{ $$ = new ASTNode(Assignment_Operator); $$->m_value = "%="; }
	| ADD_ASSIGN			{ $$ = new ASTNode(Assignment_Operator); $$->m_value = "+="; }
	| SUB_ASSIGN			{ $$ = new ASTNode(Assignment_Operator); $$->m_value = "-="; }
	| LEFT_ASSIGN			{ $$ = new ASTNode(Assignment_Operator); $$->m_value = "<<="; }
	| RIGHT_ASSIGN			{ $$ = new ASTNode(Assignment_Operator); $$->m_value = ">>="; }
	| AND_ASSIGN			{ $$ = new ASTNode(Assignment_Operator); $$->m_value = "&="; }
	| XOR_ASSIGN			{ $$ = new ASTNode(Assignment_Operator); $$->m_value = "^="; }
	| OR_ASSIGN				{ $$ = new ASTNode(Assignment_Operator); $$->m_value = "|="; }
	;

expression
	: assignment_expression							{ $$ = new ASTNode(Assignment_Expression); $$->pushChild($1); }
	| expression ',' assignment_expression			{ $$ = new ASTNode(Expression); $$->pushChild($1); $$->pushChild($3); }
	;

constant_expression
	: conditional_expression	/* with constraints */
	;

declaration
	: declaration_specifiers ';'  							{ $$ = new ASTNode(Declaration); $$->pushChild($1); } 
	| declaration_specifiers init_declarator_list ';' 		{ $$ = new ASTNode(Declaration); $$->pushChild($1); $$->pushChild($2); } 
	| static_assert_declaration								{ $$ = new ASTNode(Declaration); $$->pushChild($1); }	
	;

declaration_specifiers
	: storage_class_specifier declaration_specifiers  		{ $$ = new ASTNode(Declaration_Specifiers); $$->pushChild($1); $$->pushChild($2); }
	| storage_class_specifier 								{ $$ = new ASTNode(Declaration_Specifiers); $$->pushChild($1); }
	| type_specifier declaration_specifiers   				{ $$ = new ASTNode(Declaration_Specifiers); $$->pushChild($1); $$->pushChild($2); }
	| type_specifier										{ $$ = new ASTNode(Declaration_Specifiers); $$->pushChild($1); }
	| type_qualifier declaration_specifiers					{ $$ = new ASTNode(Declaration_Specifiers); $$->pushChild($1); $$->pushChild($2); }	
	| type_qualifier										{ $$ = new ASTNode(Declaration_Specifiers); $$->pushChild($1); } 
	| function_specifier declaration_specifiers				{ $$ = new ASTNode(Declaration_Specifiers); $$->pushChild($1); $$->pushChild($2); }
	| function_specifier									{ $$ = new ASTNode(Declaration_Specifiers); $$->pushChild($1); }
	| alignment_specifier declaration_specifiers			{ $$ = new ASTNode(Declaration_Specifiers); $$->pushChild($1); $$->pushChild($2); }
	| alignment_specifier									{ $$ = new ASTNode(Declaration_Specifiers); $$->pushChild($1); }
	;

init_declarator_list
	: init_declarator
	| init_declarator_list ',' init_declarator
	;

init_declarator
	: declarator '=' initializer
	| declarator
	;

storage_class_specifier
	: TYPEDEF	/* identifiers must be flagged as TYPEDEF_NAME */
	| EXTERN
	| STATIC
	| THREAD_LOCAL
	| AUTO
	| REGISTER
	;

type_specifier
	: VOID						{ $$ = new ASTNode(Type_Specifier); $$->m_value = "VOID"; }
	| CHAR						{ $$ = new ASTNode(Type_Specifier); $$->m_value = "CHAR"; }
	| SHORT						{ $$ = new ASTNode(Type_Specifier); $$->m_value = "SHORT"; }
	| INT						{ $$ = new ASTNode(Type_Specifier); $$->m_value = "INT"; }
	| LONG						{ $$ = new ASTNode(Type_Specifier); $$->m_value = "LONG"; }
	| FLOAT						{ $$ = new ASTNode(Type_Specifier); $$->m_value = "FLOAT"; }
	| DOUBLE					{ $$ = new ASTNode(Type_Specifier); $$->m_value = "DOUBLE"; }
	| SIGNED					{ $$ = new ASTNode(Type_Specifier); $$->m_value = "SIGNED"; }
	| UNSIGNED					{ $$ = new ASTNode(Type_Specifier); $$->m_value = "UNSIGNED"; }
	| BOOL						{ $$ = new ASTNode(Type_Specifier); $$->m_value = "BOOL"; }
	| COMPLEX					{ $$ = new ASTNode(Type_Specifier); $$->m_value = "COMPLEX"; }
	| IMAGINARY	  				{ $$ = new ASTNode(Type_Specifier); $$->m_value = "IMAGINARY"; }
	| atomic_type_specifier		{ $$ = new ASTNode(Type_Specifier); $$->pushChild($1); }
	| struct_or_union_specifier	{ $$ = new ASTNode(Type_Specifier); $$->pushChild($1); }
	| enum_specifier			{ $$ = new ASTNode(Type_Specifier); $$->pushChild($1); }
	| TYPEDEF_NAME				{ $$ = new ASTNode(Type_Specifier); $$->m_value = "TYPEDEF_NAME"; }
	;

struct_or_union_specifier
	: struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list ';'	/* for anonymous struct/union */
	| specifier_qualifier_list struct_declarator_list ';'
	| static_assert_declaration
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
	| type_specifier
	| type_qualifier specifier_qualifier_list
	| type_qualifier
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: ':' constant_expression
	| declarator ':' constant_expression
	| declarator
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator	/* identifiers must be flagged as ENUMERATION_CONSTANT */
	: enumeration_constant '=' constant_expression
	| enumeration_constant
	;

atomic_type_specifier
	: ATOMIC '(' type_name ')'
	;

type_qualifier
	: CONST
	| RESTRICT
	| VOLATILE
	| ATOMIC
	;

function_specifier
	: INLINE
	| NORETURN
	;

alignment_specifier
	: ALIGNAS '(' type_name ')'
	| ALIGNAS '(' constant_expression ')'
	;

declarator
	: pointer direct_declarator 		{ $$ = new ASTNode(Declarator); $$->pushChild($1); $$->pushChild($2); }
	| direct_declarator					{ $$ = new ASTNode(Declarator); $$->pushChild($1); }
	;

direct_declarator
	: IDENTIFIER																		{ $$ = new ASTNode(Identifier); $$->m_value = yylval.str; }				
	| '(' declarator ')'																{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($2); }
	| direct_declarator '[' ']'															{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); }
	| direct_declarator '[' '*' ']'														{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); }	
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'		{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); $$->pushChild($4); $$->pushChild($5); $$->m_value = "STATIC"; }
	| direct_declarator '[' STATIC assignment_expression ']'							{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); $$->pushChild($4); $$->m_value = "STATIC"; }
	| direct_declarator '[' type_qualifier_list '*' ']'									{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); $$->pushChild($3); $$->m_value = "*"; }
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'		{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); $$->pushChild($3); $$->pushChild($5); $$->m_value = "STATIC"; }
	| direct_declarator '[' type_qualifier_list assignment_expression ']'				{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); $$->pushChild($3); $$->pushChild($4); }	
	| direct_declarator '[' type_qualifier_list ']'										{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); $$->pushChild($3); }
	| direct_declarator '[' assignment_expression ']'									{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); $$->pushChild($3); }	
	| direct_declarator '(' parameter_type_list ')'										{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); $$->pushChild($3); }
	| direct_declarator '(' ')'															{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); }
	| direct_declarator '(' identifier_list ')'											{ $$ = new ASTNode(Direct_Declarator); $$->pushChild($1); $$->pushChild($3); }
	;

pointer
	: '*' type_qualifier_list pointer
	| '*' type_qualifier_list
	| '*' pointer
	| '*'
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;


parameter_type_list
	: parameter_list ',' ELLIPSIS
	| parameter_list
	;

parameter_list
	: parameter_declaration
	| parameter_list ',' parameter_declaration
	;

parameter_declaration
	: declaration_specifiers declarator
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;

identifier_list
	: IDENTIFIER							{ $$ = new ASTNode(Identifier); $$->m_value = yylval.str; }
	| identifier_list ',' IDENTIFIER		{ $$ = new ASTNode(Identifier_List); $$->pushChild($1); ASTNode* temp = new ASTNode(Identifier); temp->m_value = yylval.str; $$->pushChild(temp);}
	;

type_name
	: specifier_qualifier_list abstract_declarator		
	| specifier_qualifier_list
	;

abstract_declarator
	: pointer direct_abstract_declarator
	| pointer
	| direct_abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
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
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: '{' initializer_list '}'
	| '{' initializer_list ',' '}'
	| assignment_expression
	;

initializer_list
	: designation initializer
	| initializer
	| initializer_list ',' designation initializer
	| initializer_list ',' initializer
	;

designation
	: designator_list '='
	;

designator_list
	: designator
	| designator_list designator
	;

designator
	: '[' constant_expression ']'
	| '.' IDENTIFIER
	;

static_assert_declaration
	: STATIC_ASSERT '(' constant_expression ',' STRING_LITERAL ')' ';'
	;

statement 
	: labeled_statement					{ $$ = new ASTNode(Statement); $$->pushChild($1); }
	| compound_statement				{ $$ = new ASTNode(Statement); $$->pushChild($1); }
	| expression_statement				{ $$ = new ASTNode(Statement); $$->pushChild($1); }
	| selection_statement				{ $$ = new ASTNode(Statement); $$->pushChild($1); }
	| iteration_statement				{ $$ = new ASTNode(Statement); $$->pushChild($1); }
	| jump_statement					{ $$ = new ASTNode(Statement); $$->pushChild($1); }
	;

labeled_statement
	: IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'							{ $$ = new ASTNode(Compound_Statement); $$->m_value = ""; }
	| '{'  block_item_list '}'			{ $$ = new ASTNode(Compound_Statement); $$->pushChild($2); }
	;

block_item_list
	: block_item						{ $$ = new ASTNode(Block); $$->pushChild($1); }
	| block_item_list block_item		{ $$ = $1; $$->pushChild($2); }
	;

block_item
	: declaration						{ $$ = $1; }
	| statement							{ $$ = $1; }
	;

expression_statement
	: ';'								{ $$ = NULL; }
	| expression ';'					{ $$ = $1; }
	;

selection_statement
	: IF '(' expression ')' statement ELSE statement
	| IF '(' expression ')' statement
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	| FOR '(' declaration expression_statement ')' statement
	| FOR '(' declaration expression_statement expression ')' statement
	;

jump_statement
	: GOTO IDENTIFIER ';'				{ $$ = new ASTNode(Jump_Statement); $$->m_value = "GOTO IDENTIFIER"; } // incomplete; how to fetch string correspongind to identifier???
	| CONTINUE ';'						{ $$ = new ASTNode(Jump_Statement); $$->m_value = "CONTINUE"; }	
	| BREAK ';'							{ $$ = new ASTNode(Jump_Statement); $$->m_value = "BREAK"; }
	| RETURN ';'						{ $$ = new ASTNode(Jump_Statement); $$->m_value = "RETURN"; }
	| RETURN expression ';'				{ $$ = new ASTNode(Jump_Statement); $$->m_value = "RETURN"; $$->pushChild($2); }
	;

translation_unit
	: external_declaration 							{ $$ = new ASTNode(Begin); $$->pushChild($1); root = $$; }
	| translation_unit external_declaration 		{ $$ = new ASTNode(Begin); $$->pushChild($1); $$->pushChild($2); root = $$; }
	;

external_declaration
	: function_definition							{ $$ = new ASTNode(External_Declaration); $$->pushChild($1); }
	| declaration									{ $$ = new ASTNode(External_Declaration); $$->pushChild($1); }		
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement		{ $$ = new ASTNode(Function_Definition); $$->pushChild($1); $$->pushChild($2); $$->pushChild($3); $$->pushChild($4); }
	| declaration_specifiers declarator compound_statement						{ $$ = new ASTNode(Function_Definition); $$->pushChild($1); $$->pushChild($2); $$->pushChild($3); }
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

%%
#include <stdio.h>

void yyerror(const char *s)
{
	fflush(stdout);
	fprintf(stderr, "*** %s\n", s);
}
