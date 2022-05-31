#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "token.h"

#define list_token_head(pos) list_for_each_entry(pos, &token_head->list)

#define MAX_IDENTIFIER_SIZE 32

#define is_maiusc(x) (in_range('A', x, 'Z'))
#define is_minusc(x) (in_range('a', x, 'z'))
#define is_number(x) (in_range('0', x, '9'))


/* These constant are used in 'token_add()' to define if the value inserted
 * in token_head is a token or an identifier \ constant */
#define TOKEN 0
#define DATA 1

/* Used only in 'read_word()' */
#define COMMENT_LINE 1
#define COMMENT_STAR 2


static void init_tokenize ();
//static Ident *array_ident_head (Ident *pointer_to_array);
static void read_file (FILE *in);
static int read_word (FILE *in, char dest[MAX_IDENTIFIER_SIZE]);
static bool delete_comment (FILE *in, char c);
static int is_letter_keyword (char letter);
static bool add_keyword (char word[MAX_IDENTIFIER_SIZE]);
static int is_keyword (char word[MAX_IDENTIFIER_SIZE]);
static bool add_constant (char word[MAX_IDENTIFIER_SIZE]);
static bool is_constant_number (char word[MAX_IDENTIFIER_SIZE]);
static bool add_ident_to_token_head (char word[MAX_IDENTIFIER_SIZE]);
static Ident *add_ident (char word[MAX_IDENTIFIER_SIZE]);
static void conflicting_hash (Ident *new);
static Token *add_token (const QWORD token_value, const int CONSTANT);


static Token *last_token;
static Ident *ident_head;
static Token *token_head;

struct token_struct token_list[] = {

	{ T_IDENT, "-IDENT-" },
	{ T_CONSTANT, "-CONSTANT-" },
	{ CHAR, "char" },
	{ FLOAT, "float" },
	{ INT, "int" },
	{ LONG, "long" },
	{ SHORT, "short" },
	{ VOID, "void" },
	{ CONST, "const" },
	{ SIGNED, "signed" },
	{ STATIC, "static" },
	{ UNSIGNED, "unsigned" },
	{ ASSIGN, "=" },
	{ SHL, "<<" },
	{ SHR, ">>" },
	{ ADD, "+" },
	{ SUB, "-" },
	{ MOLTIPLICATE, "*" },
	{ DIVIDE, "/" },
	{ BREAK, "break" },
	{ CONTINUE, "continue" },
	{ IF, "if" },
	{ ELSE, "else" },
	{ RETURN, "return" },
	{ WHILE, "while" },
	{ BRACKET_OPEN, "[" },
	{ BRACKET_CLOSE, "]" },
	{ PAR_OPEN, "(" },
	{ PAR_CLOSE, ")" },
	{ BRACE_OPEN, "{" },
	{ BRACE_CLOSE, "}" },
	{ COMMA, ","},
	{ SEMICOLON, ";" },
	{ SPACE, " " },
	{ TAB, "\t" },
	{ CR, "\n" },
	{ DEFINE, "#define" },
	{ EF, "EOF" },
	{ 0,    NULL}
};

void tokenize (FILE *in, Token **return_token_head, Ident **return_ident_head)
{

//	Ident *id_head;

	init_tokenize();
	read_file(in);
	//array_ident_head(id_head);

	*return_ident_head = ident_head;
	*return_token_head = token_head;
}

/*
static Ident *array_ident_head (Ident *pointer_to_array)
{
	Ident *id;
	Ident *id_head[n_idents];
	int i;

	i = 0;
	list_for_each_entry(id, &token_head->list) {
		id_head[i] = id;
		i++;
	}
	pointer_to_array = id_head;
}
*/
void delete_spaces (Token *token_head)
{
	Token *pos;
	int tok;

	pos = malloc(sizeof(Token));
	list_for_each_entry(pos , &token_head->list) {
		tok = pos->tok;

        //newlines not delete because we need it for error's message
        //later on.
		if (tok == SPACE || tok == TAB)
			list_del(&pos->list);
	}
}

static void init_tokenize ()
{
	token_head = oalloc(sizeof(Token));
	last_token = oalloc(sizeof(Token));
	ident_head = oalloc(sizeof(Ident));

	INIT_LIST_HEAD(&token_head->list);
	INIT_LIST_HEAD(&ident_head->list);
}


static void read_file (FILE *in)
{
	char word[MAX_IDENTIFIER_SIZE];
	int i;
	int ret_read_word;

	bzero(word, MAX_IDENTIFIER_SIZE);
	for (i = 0; !feof(in); i++) {
		ret_read_word = read_word(in, word);

		if (add_keyword(word))
			bzero(word, MAX_IDENTIFIER_SIZE);
		else if (add_constant(word))
			bzero(word, MAX_IDENTIFIER_SIZE);
		else if (add_ident_to_token_head(word))
			bzero(word, MAX_IDENTIFIER_SIZE);

		add_token(ret_read_word, TOKEN);
	}
}

static int read_word (FILE *in, char dest[MAX_IDENTIFIER_SIZE])
{
	int i;
	char c;

	for (i = 0; !feof(in) && i < MAX_IDENTIFIER_SIZE; i++) {
		c = getc(in);

		if (delete_comment(in, c))
			return 0;

		if (is_number(c) || is_maiusc(c) || is_minusc(c) || c == '_') {
			dest[i] = c;
			continue;
		}
		return is_letter_keyword(c);
	}

	if (i >= MAX_IDENTIFIER_SIZE)
		fatal("constant or identifier too long");
	return EF;
}

static bool delete_comment (FILE *in, char c)
{
	bool comment_line = 0;
	bool comment_star = 0;

	if (c == '/') {
		c = getc(in);
		if (c == '/')
			comment_line = 1;
		else if (c == '*')
			comment_star = 1;
		else {
			fseek(in, -1, SEEK_CUR);
			return FAIL;
		}
	}

	while (comment_line) {
		c = getc(in);
		if (c == '\n')
			return SUCCESS;
	}

	while (comment_star) {
		c = getc(in);
		if (c == '*') {
			c = getc(in);
			if (c == '/')
				return SUCCESS;
			fseek(in, -1, SEEK_CUR);
		}
	}
	return FAIL;
}

static int is_letter_keyword (char letter)
{
	switch (letter) {
	case ' ': return SPACE;
	case '\t': return TAB;
	case '\n': return CR;
	case '=': return ASSIGN;
	case '+': return ADD;
	case '-': return SUB;
	case '*': return MOLTIPLICATE;
	case '/': return DIVIDE;
	case '(': return PAR_OPEN;
	case ')': return PAR_CLOSE;
	case '[': return BRACKET_OPEN;
	case ']': return BRACKET_CLOSE;
	case ',': return COMMA;
	case '{': return BRACE_OPEN;
	case '}': return BRACE_CLOSE;
	case ';': return SEMICOLON;

	case -1: return EF;

	default:
		  return FAIL;
	};
}

static bool add_keyword (char word[MAX_IDENTIFIER_SIZE])
{
	int ret;

	ret = is_keyword(word);
	if (ret) {
		add_token(ret, TOKEN);
		return SUCCESS;
	}
	return FAIL;
}

static int is_keyword (char word[MAX_IDENTIFIER_SIZE])
{
	int i;
	/*After 'EF', they are no more keywords of C language, but just tokens */
	for (i = CHAR - 1; token_list[i].value < MAX_TOKEN; i++)
		if (!strcmp(token_list[i].word, word))
			return token_list[i].value;
	return FAIL;
}

static bool add_constant (char word[MAX_IDENTIFIER_SIZE])
{
	if (is_constant_number(word)) {
		add_token(T_CONSTANT, TOKEN);
		add_token(atol(word), DATA);
		return SUCCESS;
	}
	return FAIL;
}

static bool is_constant_number (char word[MAX_IDENTIFIER_SIZE])
{
	if (atol(word))
		return SUCCESS;

	return FAIL;
}

static bool add_ident_to_token_head (char word[MAX_IDENTIFIER_SIZE])
{
	Ident *new;

	if (word[0] == '\0')
		return FAIL;

	new = add_ident(word);
	add_token(T_IDENT, TOKEN);
	add_token((QWORD)new, DATA);

	return SUCCESS;
}

static Ident *add_ident (char word[MAX_IDENTIFIER_SIZE])
{
	Ident *new;
	Ident *tmp;

	new = oalloc(sizeof(Ident));

	new->hash = hash(word);
	new->name = malloc(strlen(word) + 1);
	strcpy(new->name, word);

	conflicting_hash(new);

	//Sort adresses in ascending order
	list_for_each_entry(tmp, &ident_head->list) {
		if (new->hash < tmp->hash)
			break;
	}

	list_add_tail(&new->list, &tmp->list);
//	list_add(&new->list, &ident_head->list);
	return new;
}

static void conflicting_hash (Ident *new)
{
	Ident *id;

	list_for_each_entry(id, &ident_head->list) {
		if (id->hash == new->hash && strcmp(id->name, new->name))
				fatal("Conflicting hashes. ('%s' and '%s'). Compiler's fault",
				      id->name, new->hash);
	}
}

static Token *add_token (const QWORD token_value, const int CONSTANT)
{
	Token *new;

	if (!token_value)
		return NULL;

	new = oalloc(sizeof(Token));

	if (CONSTANT == TOKEN) {
		new->tok = token_value;
	} else {
		if (last_token->tok == T_CONSTANT)
			new->constant = token_value;
		else
			new->ident = (Ident *)token_value;
		//return last_token;
	}

	list_add_tail(&new->list, &token_head->list);
	//list_add(&new->list, &token_head->list);

	last_token = new;
	return new;
}
