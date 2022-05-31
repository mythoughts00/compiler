#ifndef IR_H
#define IR_H

#include "token.h"

typedef struct ir {
	unsigned int instruction;

	struct reg *dest;
	struct reg *op1;
    struct reg *op2;

	unsigned int line_count;
	struct list_head list;
} Ir;

typedef struct reg {
	size_t register_address;
	union {
		Ident *id;
		size_t value;
	};
	bool address; //Is 1 if the value of 'value' should be deferenced

	struct list_head list;
} Reg;

/* There are infinite general purpose register. Only REG_ZERO has a purpose,
 * which, you can guess, it has only one value. 0.
 * P.S.
 * I don't think is elegant to put it here, because it for sure help the
 * analizer, but is not clean at this stage of the compilation
 */
#define REG_ZERO 1

enum { IR_OR, IR_AND, IR_NOT, IR_XOR };

enum { IR_ADD = IR_XOR + 1, IR_SUB, IR_MOLTIPLICATE, IR_DIVIDE, IR_SHL, IR_SHR };

enum { IR_LOAD = IR_SHR + 1, IR_STORE };

enum { JUMP = IR_STORE + 1, IR_FUNCTION };

#endif
