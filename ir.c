/* !!!!!!!!!!!!!!!! IDEA !!!!!!!!!!!!!!!!!!
 * Building kind of a python dictionary with the address of the identifier
 * as an index, and tha actual value is the register pointer
 * */

#include <string.h> //used only for bzero()
#include "token.h"
#include "ir.h"


#define ABSOLUTE_VALUE 1
#define VARIABLE 2

#define INSTRUCTION 1
#define DEST 2
#define OP1 3

static void init_ir ();
static void modify_line (const int CONSTANT, QWORD value);
static void ir_block_controller (Token *pos);
static void ir_arithmetic (Token *pos);
static void ir_identifier (Token *pos);
static Reg *add_reg (const int CONSTANT, Ident *id);
static Reg *return_reg (const size_t register_address);
static Ir *add_ir_line ();
static bool line_complete ();
static void void_line ();

extern struct status_source status;

static Ir *ir_head;
static Ir *ir_line;
static Reg *reg_head;

static unsigned int line_count = 0;
static unsigned int last_reg = REG_ZERO + 1;
static Token *next;

/* I want that the intermediate language is running along 'parser()',
 * this way i have already built all the status and flags of the code */

static void init_ir ()
{
    ir_head = oalloc(sizeof(Ir));
    ir_line = oalloc(sizeof(Ir));
    reg_head = oalloc(sizeof(Reg));

	INIT_LIST_HEAD(&ir_head->list);
	INIT_LIST_HEAD(&reg_head->list);
}

void ir (Token *pos)
{
	next = next_token(pos);

	if (is_identifier(pos))
		ir_identifier(pos);

    else if (is_arithmetic(pos))
		ir_arithmetic(pos);

    else if (is_block_controller(pos))
		ir_block_controller(pos);

    if (line_complete())
        add_ir_line();
}

static void ir_block_controller (Token *pos)
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

static void ir_arithmetic (Token *pos)
{
	switch (pos->tok) {
	case SHL: modify_line(INSTRUCTION, IR_SHL); break;
	case SHR: modify_line(INSTRUCTION, IR_SHR); break;
	case ADD: modify_line(INSTRUCTION, IR_ADD); break;
	case SUB: modify_line(INSTRUCTION, IR_SUB); break;
	case MOLTIPLICATE: modify_line(INSTRUCTION, IR_MOLTIPLICATE); break;
	case DIVIDE: modify_line(INSTRUCTION, IR_DIVIDE); break;
	case ASSIGN: modify_line(INSTRUCTION, IR_STORE);

	default:
		fatal("Unknown operator");
	};
}

static void modify_line (const int CONSTANT, size_t value)
{
	if (CONSTANT == INSTRUCTION)
		ir_line->instruction = (unsigned int)value;

    else if (CONSTANT == DEST)
		ir_line->dest = (Reg *)value;

    else if (CONSTANT == OP1)
		ir_line->op1 = (Reg *)value;
}

static void ir_identifier (Token *pos)
{
	Ident *id = pos->ident;
	Reg *r;

	if (status.ident_is_function) {// Means is function declaration
        r = add_reg(VARIABLE, id);
        void_line();
        modify_line(INSTRUCTION, IR_FUNCTION);
        modify_line(DEST, r);
        add_ir_line();
        return;
	}

	r = add_reg(VARIABLE, id);

	if (ir_line->op1 == NULL)
		ir_line->op1 = r;
    else {
        ir_line->op2 = r;

    }
}

static Ir *add_ir_line ()
{
	Ir *new;

	new = oalloc(sizeof(Ir));

	new->instruction = ir_line->instruction;
	new->dest = ir_line->dest;
	new->op1 = ir_line->source;
	new->line_count = ir_line->line_count;

	list_add_tail(&new->list, &ir_head->list);
    void_line();

	++line_count;
	ir_line->line_count = line_count;
}

static bool line_complete ()
{
	if (ir_line->instruction && ir_line->dest && ir_line->op1)
		return SUCCESS;

	return FAIL;
}

static void void_line ()
{
	bzero(ir_line, sizeof(Ir));
}

static Reg *add_reg (const int CONSTANT, Ident *id)
{
	Reg *new;
	Reg *prev;

	new = oalloc(sizeof(Reg));
	list_add_tail(&new->list, &reg_head->list);

	if (CONSTANT == VARIABLE)
		new->id = id;
	else
		new->value = (size_t)id;

	prev = list_prev_entry(new);
    //Register are not reused. It just build new one. The analizer will
    //handle the reusage of finite registers. Here we assume are infinite
	if (reg_head == prev)
		new->register_address = REG_ZERO + 1;
	else
		new->register_address = prev->register_address + 1;

	return new;
}
