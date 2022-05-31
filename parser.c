#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

#include "token.h"

struct stack_block {
    Token *t;
	int val;
	int close_type;
	struct stack_block *prev;
};

static void do_identifier ();
static void do_arithemtic ();
static void do_brackets (int val);
static void do_parenthesis (int val);
static void do_semicolon ();
static void do_specifier ();
static void do_qualifier ();
static void do_flow_controller ();
static void do_block_controller ();
static void new_block_value (const int brace_val);
static void push (int close_type);
static void pop (const int val);
static int expect (int num, ...);
static void print_stack();

static Token *token_head;
static Token *pos;
static Token *next;

static struct stack_block *sp;

unsigned int current_line;

struct status_source status = {

	.while_block = false,
	.if_block = false,
	.parenthesis = false,
	.brace = false,
	.bracket = false,

	.in_statement = false,
	.if_statement = false,
	.while_statement = false,
	.in_function = false,
	.in_function_declaration = false,
	.semicolon_in_stack = false
};

void parser (Token *head)
{
	token_head = head;
    current_line = 1;

	sp = oalloc(sizeof(struct stack_block));

	list_for_each_entry(pos, &token_head->list) {
		next = next_token(pos);

		if (is_identifier(pos)) {
			do_identifier();

		} else if (is_arithmetic(pos)) {
			do_arithemtic();

		} else if (is_block_controller(pos)) {
			do_block_controller();

		} else if (is_flow_controller(pos)) {
			do_flow_controller();

		} else if (is_constant(pos)) {
			do_identifier(); //Expect the same tokens as identifier

		} else if (is_specifier(pos)) {
			do_specifier();

		} else if (is_qualifier(pos)) {
			do_qualifier();
            //
        //counts were is in the file then delete the token 'CR'
        } else if (pos->tok == CR) {
            ++current_line;
            list_del(&pos->list);

		} else if (pos->tok == EF) {
            if (sp->close_type)
			    fatal("Missing '}'");

			return;

		} else {
			fatal("Unknown token %t", pos);
		}
    //    print_stack();
	}
}

static void do_specifier ()
{
	fatal("Should not be specifiers. the global function 'identifier()'"
	      "should've take care of that.\n"
	      "If you think about cast, i'm sorry, but is not handled");
}

static void do_qualifier ()
{
	fatal("Should not be qualifiers. the global function 'identifier()'"
	      "should've take care of that.\n"
	      "If you think about cast, i'm sorry, but is not handled");
}

static void do_arithemtic()
{
	if (!expect(3, T_IDENT, T_CONSTANT, PAR_OPEN))
		fatal("Expected identifier or constant");
}

static void do_identifier ()
{
	Ident *id;

	if (is_flow_controller(next))
		fatal("Expected ';'");

	if (expect(3, BRACE_CLOSE, BRACE_OPEN, PAR_CLOSE))
		fatal("Expected ';'");

	if (expect(1, PAR_OPEN)) {
		status.ident_is_function = true;

		id = pos->ident;
		if (!(id->mask_type |= FUNCTION))
			fatal("%i: Bad contruction of function in 'identifier()'", id);

		if (id->args > MAX_FUNCTION_ARGUMENTS)
			fatal("%i has to %d arguments. MAX = %d",
			      id, id->args, MAX_FUNCTION_ARGUMENTS);
	}

	else if (!status.semicolon_in_stack &&
		 !status.in_function_declaration &&
		 !status.in_statement) {
		push(SEMICOLON);
		status.semicolon_in_stack = true;
	}
}

static void do_flow_controller ()
{
	if (status.in_statement)
		fatal("Flow controllers inside statement");

	switch (pos->tok) {
	case BREAK:
	case CONTINUE:
		if (!expect(1, SEMICOLON))
			fatal("Expected ';'");

		if (!status.while_block)
			fatal("'break' or 'continue' not inside a recursion");
		break;

	case IF:
		if (!expect(1, PAR_OPEN))
			fatal("Expected '('");
		status.if_statement = true;
		status.in_statement = true;
		break;

	case WHILE:
		if (!expect(1, PAR_OPEN))
			fatal("Expected '('");
		status.while_statement = true;
		status.in_statement = true;
		break;

	case RETURN:
		if (!is_constant(next) && !is_identifier(next))
			fatal("Expected constant or identifier");

		if (!status.in_function)
			fatal("%t used outside a funciton", pos);
		break;

	case COMMA:
		return;
	};
}

static void do_block_controller ()
{
	switch (pos->tok) {

	case BRACKET_OPEN:
		do_brackets(BRACKET_OPEN);
		return;

	case BRACKET_CLOSE:
		do_brackets(BRACKET_CLOSE);
		return;

	case BRACE_OPEN:
		new_block_value(BRACE_OPEN);
		return;

	case BRACE_CLOSE:
		new_block_value(BRACE_CLOSE);
		return;

	case PAR_OPEN:
		do_parenthesis(PAR_OPEN);
		return;

	case PAR_CLOSE:
		do_parenthesis(PAR_CLOSE);
		return;

	case SEMICOLON:
		do_semicolon();
	};
}

static void do_brackets (int val)
{
	if (val == BRACKET_CLOSE) {

		if (!is_arithmetic(next) && !expect(2, SEMICOLON, PAR_CLOSE))
			fatal("Expected ';' or ')'");

		if (sp->close_type != BRACKET_CLOSE)
			fatal("Missing ']'");

		if (status.bracket == false)
			fatal("Closing an unopened bracket");
		status.bracket--;
		pop(BRACKET_CLOSE);

	} else {
		if (is_flow_controller(next))
			fatal("Bracket around flow controller");

		if (is_arithmetic(next))
			fatal("Expected identifier or constant");
		push(BRACKET_CLOSE);
		status.bracket++;
	}
}

static void do_semicolon ()
{
	if (sp->close_type != SEMICOLON) {
		if (sp->close_type == BRACE_CLOSE)
			fatal("Missing '}'");
		fatal("A statement has not be closed correctly");
	}

	if (sp->val == IF)
		status.if_block--;

	if (sp->val == WHILE)
		status.while_block--;

	//if (status.semicolon_in_stack)
	//	status.semicolon_in_stack = 0;

	pop(SEMICOLON);

	//if (next->tok != BRACE_CLOSE)
		//push(SEMICOLON);
	//else
		status.semicolon_in_stack = false;

}

static void do_parenthesis (int val)
{
	if (is_arithmetic(next))
		fatal("%t: Expected identifier or constant");

	if (is_flow_controller(next))
		fatal("Error to put parenthesis on flow controllers");

	if (val == PAR_CLOSE) {
		if (!status.parenthesis)
			fatal("No parenthesis to close");

		if (status.if_statement) {
			status.if_block++;

		} else if (status.while_statement) {
			status.while_block++;

		//If it is a prototype
		} else if (status.in_function_declaration) {
			if (next->tok == SEMICOLON)
				status.in_function_declaration = false;
		}
		pop(PAR_CLOSE);
		status.parenthesis--;

	} else {
		if (status.ident_is_function) {
			status.ident_is_function = false;
			status.in_function_declaration = true;
		}
		push(PAR_CLOSE);
		status.parenthesis++;
	}
}

static void new_block_value (const int brace_val)
{
	if (is_arithmetic(next))
		fatal("Expected identifier or constant");

	if (brace_val == BRACE_CLOSE) {
		if (!status.brace)
			fatal("Closing a brace which is not opened");

		if (sp->close_type != BRACE_CLOSE) {
			fatal("Missing '}'");
        }

		if (sp->val == IF)
			status.if_block--;

		if (sp->val == WHILE)
			status.while_block--;

		if (sp->val == T_IDENT && status.brace == true)
			status.in_function = false;

		pop(BRACE_CLOSE);
		status.brace--;

	} else {
		if (status.in_function_declaration) {
			status.in_function_declaration = false;
			status.in_function = true;
		}
		push(BRACE_CLOSE);
		status.brace++;
		return;
	}
}

static void push (int close_type)
{
	struct stack_block *new;

	new = malloc(sizeof(struct stack_block));

    new->t = pos;
	new->val = pos->tok;
	new->close_type = close_type;
	new->prev = sp;
	sp = new;
}

static void pop (const int val)
{
	if (!sp->prev)
		fatal("Trying to pop an empty stack. Watch the code");

	if (sp->close_type != val)
		fatal("Trying to pop %d, which is not at the top of the stack", val);

	sp = sp->prev;
}

static void print_stack()
{
    struct stack_block *cur = sp;

    for (; cur->prev; cur = cur->prev) {
        puts("(");
        printt(sp->t);
        printf(", %d) -", cur->close_type);
        puts("\t");
    }
    puts("\n");

}

static int expect (int num, ...)
{
	va_list arguments;
	int arg;
	int i;

	va_start (arguments, num);

	for (i = 0; i < num; i++ ) {
		arg = va_arg(arguments, int);
		if (next->tok == arg)
			return 1;
		else
			continue;
	}
	va_end(arguments);
	return 0;
}
