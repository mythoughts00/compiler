#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include "list.h"

#define FAIL 0
#define SUCCESS 1

#define true 1
#define false 0

/* These constant are used in 'token_add()' to define if the value inserted
 * in token_head is a token or an identifier/constant */
#define TOKEN 0
#define in_range(a, x, b) (((a) <= (x) && (x) <= (b)) ? 1 : 0)


/*Most significant bit of 'ident->mask_type' is reserved for funciton */
#define FUNCTION (1 << 30) //Used in 'identifier.c' and 'ir.c'
#define MAX_FUNCTION_ARGUMENTS 4 //Used in 'identifier.c' and 'parser.c'

/* I know, name is fucking shit dog, but for the moment we keep this */
#define QWORD size_t
#define oalloc(p) calloc(1, p)

struct token_struct {
	int value;
	char *word;
};

typedef struct ident {
	size_t hash;
	char *name;
	unsigned int mask_type;
/* Tells in which block the variable has been declared */
	int block;
/* How many arguments take the function. (so 0 when is a normal varaible) */
	int args;
	struct list_head list;

} Ident;

typedef struct token {
	int tok;
	union {
		size_t constant;
		Ident *ident;
	};
	struct list_head list;
	// used in 'priority' and 'ir'
	struct token *parent;
	struct token *left;
	struct token *right;
} Token;

struct status_source {
	unsigned int while_block;
	unsigned int if_block;
	unsigned int parenthesis;
	unsigned int brace;
	unsigned int bracket;

	bool in_statement;
	bool if_statement;
	bool while_statement;
	bool in_function;
	bool in_arguments_declaration;
    bool in_global_scope;
	bool semicolon_in_stack;
	bool ident_is_function;
};

/* The order of enumeration ***MUST NOT BE CHANGED*** becuase their value, are
 * used to know the priority of operators too.
 */
enum {T_IDENT = 1, T_CONSTANT}; //2

enum { CHAR = T_CONSTANT + 1, FLOAT, INT, LONG, SHORT, VOID }; //8

enum { CONST = VOID + 1, SIGNED, STATIC, UNSIGNED };//12

enum { ASSIGN = UNSIGNED + 1, SHL, SHR,  ADD, SUB, MOLTIPLICATE, DIVIDE }; //19

enum { BREAK = DIVIDE + 1, CONTINUE, IF, ELSE, RETURN, WHILE }; //25

enum { BRACKET_OPEN = WHILE + 1, BRACKET_CLOSE, PAR_OPEN, PAR_CLOSE,
       BRACE_OPEN, BRACE_CLOSE, COMMA, SEMICOLON }; //33

enum { SPACE = SEMICOLON + 1, TAB, CR }; //36

enum { DEFINE = CR + 1 , EF}; //38

#define MAX_TOKEN EF

#define is_specifier(t) in_range(CHAR, t->tok, VOID)
#define is_qualifier(t) in_range(CONST, t->tok, UNSIGNED)
#define is_flow_controller(t) in_range(BREAK, t->tok, WHILE)
#define is_arithmetic(t) in_range(ASSIGN, t->tok, DIVIDE)
#define is_block_controller(t) in_range(BRACKET_OPEN, t->tok, SEMICOLON)
#define is_identifier(t) in_range(T_IDENT, t->tok, T_IDENT)
#define is_constant(t) in_range(T_CONSTANT, t->tok, T_CONSTANT)

#define next_token(t) list_next_entry(t)

void __fatal (const char *func, char *msg, ...);
#define fatal(msg, ...) __fatal(__func__, msg, ##__VA_ARGS__)

/* Prototypes; If not specified by the comment,
 * the function is used only by 'main()'*/

void printt (Token *t);
size_t hash (char *s); //used in 'token.c'
void print_token_head (Token *token_head);
void print_ident_head (Ident *ident_head);
void tokenize (FILE *in, Token **return_token_head, Ident **return_ident_head);
void delete_spaces (Token *token_head);
void identifier (Token **tok_head, Ident **id_head);
void priority (Token *token_head);
void parser (Token *head);
void ir (Token *pos);
#endif
