/*
 * This file delete token that are qualifiers or specifiers, moving the
 * information in '-IDENT-' token. We do this so we shrink the size of
 * 'token_head', and all the information about that variable is condensed
 * in 1 token (the identifier token), so is more manageble
 * P.S.
 *  I will do the same for constants. I'm doing it here because... i don't know
 *  were to do that XD
 */


#include <stdio.h>
#include <stdbool.h>
#include "token.h"

#define MASK_ON_SPECIFIER (UINT_MAX >> ((sizeof(int) * 8) - VOID))

static void build_identifier ();
static void mask_ident ();
static bool do_qualifier_and_specifier ();
static void check_error_specifier ();
static void check_error_qualifier ();
static bool specifier_already_declared ();
static bool is_identifier_mask_bit (const int bit);
static bool is_bit_on (const int var, const int shift);
static bool ident_exist (Ident *id);
static Ident *find_ident (Ident *id);
static void shrink_constant ();


static Token *token_head;
static Token *pos;

static Ident *ident_head;
static Ident *function_declaration;
static int identifier_mask = 0;

static int block = 0;


void identifier (Token **tok_head, Ident **id_head)
{
	token_head = *tok_head;
	ident_head = *id_head;
	list_for_each_entry(pos, &token_head->list) {
		//if (is_specifier(pos) || is_qualifier(pos)) {
		if (do_qualifier_and_specifier()){
			mask_ident();
			list_del(&pos->list);


        } else if (pos->tok == T_CONSTANT) {
            shrink_constant();

		} else if (pos->tok == T_IDENT) {
            pos = list_next_entry(pos);
			build_identifier();

		} else if (pos->tok == BRACE_OPEN) {
			block++;

		} else if (pos->tok == BRACE_CLOSE) {
			block--;

		/* I personally dont like the position of this statement */
		} else if (pos->tok == PAR_CLOSE) {
			function_declaration = NULL;
		}
	}
	*tok_head = token_head;
	*id_head = ident_head;
}

static bool do_qualifier_and_specifier ()
{
	if (is_specifier(pos)) {
		check_error_specifier();
		return SUCCESS;

	} else if (is_qualifier(pos)) {
		check_error_qualifier();
		return SUCCESS;

	} else {
		return FAIL;
	}
}

static void mask_ident ()
{
	identifier_mask |= (1 << (pos->tok - 1));
}

static void check_error_specifier ()
{
	if (specifier_already_declared())
		fatal("Multiple specifiers in same identifier");
}

static bool specifier_already_declared ()
{
	if (identifier_mask & MASK_ON_SPECIFIER)
		return SUCCESS;
	return FAIL;
}

static bool is_bit_on (const int var, const int shift)
{
	int i = 1 << shift;

	if (var & i)
		return SUCCESS;
	return FAIL;
}

static bool is_identifier_mask_bit (const int bit)
{
	if (is_bit_on(identifier_mask, bit))
		return SUCCESS;
	return FAIL;
}

static void check_error_qualifier ()
{
	Token *next = list_next_entry(pos);
	if (is_identifier(next) && !specifier_already_declared())
		fatal("No specifier has been declared");

	if (is_identifier_mask_bit(SIGNED) &&
	    is_identifier_mask_bit(UNSIGNED))
		fatal("Identifier declared 'signed' and 'unsigned'");
}
static bool declaring_ident ()
{
	if (identifier_mask)
		return SUCCESS;
	return FAIL;
}

static void build_identifier ()
{
	/* Ident structure is already built by the function "tokenize()".
	 * The only fields already done is "name" and "hash". */
	Ident *id = pos->ident;
	Token *next = list_next_entry(pos);
	Token *prev = list_prev_entry(pos);

	if (!declaring_ident()) {
		if (!ident_exist(id)) {
			fatal("ident %i not declared", id);

		} else {
			pos->ident = find_ident(id);
			list_del(&id->list);
            pos->tok = prev->tok;
            list_del(&prev->list);
			return;
		}
	}
	id->mask_type = identifier_mask;
	id->block = block;

	if (function_declaration)
		function_declaration->args++;

	if (next->tok == PAR_OPEN) {
		function_declaration = id;
		id->mask_type |= FUNCTION;

	}
	identifier_mask = 0;

    pos->tok = prev->tok;
    list_del(&prev->list);
}

static bool ident_exist (Ident *id)
{
	Ident *cur;

	list_for_each_entry(cur, &ident_head->list) {
		if (cur->hash == id->hash && cur->block == id->block)
			return SUCCESS;
	}
	return FAIL;
}

static Ident *find_ident (Ident *id)
{
	Ident *id_pos;
	list_for_each_entry(id_pos, &ident_head->list)
		if (id_pos->hash == id->hash && id_pos->mask_type != 0)
			return id_pos;

	fatal("%i is not declared", id);

	return NULL;
}

static void shrink_constant ()
{
	Token *next = list_next_entry(pos);

    pos->constant = next->constant;
    list_del(&next->list);
}
