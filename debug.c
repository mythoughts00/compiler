#include <stdio.h>
#include <stdlib.h>

#include "token.h"


extern struct token_struct token_list[];
static int decimal_to_binary (int n);
static void print_token_value (const int tok);


void printt (Token *t)
{
	if (t)
		print_token_value(t->tok);
	else
		printf("'NULL'");
}

static void print_token_value (const int tok)
{
	printf("%s", token_list[tok - 1].word);
}

void print_token_head (Token *token_head)
{
	Token *t;
	Token *head = token_head;

	putchar('\n');
	printf("Debugging 'token_head'\n");
	list_for_each_entry(t, &head->list) {
        printf("%p --> '", t);
        printt(t);
        puts("'");
        /*

		if (t->tok == T_IDENT) {
			printf("'%s'", t->ident->name);

		} else if (t->tok == T_CONSTANT) {
			printf("|%ld|", t->constant);
		} else {
			if (t->left) {
				printt(t->left);
				printf("<--");
			}

			printt(t);

			if (t->right) {
				printf("-->");
				printt(t->right);
			}
		}
        */
	}
	putchar('\n');
}

void print_ident_head (Ident *ident_head)
{
	Ident *id;
	
	id = malloc(sizeof(Ident));

	putchar('\n');
	printf("Debugging 'ident_head'\n");
	list_for_each_entry(id, &ident_head->list) {
		putchar('{');

		printf("'%s', ", id->name);
		decimal_to_binary(id->mask_type);
		printf("%lu, %d, %d", id->hash, id->block, id->args);

		putchar('}');
		putchar(' ');
	}
	putchar('\n');
}

static int decimal_to_binary (int n)
{
	int a[32], i;

	for(i = 0; n > 0; i++) {
		a[i] = n % 2;
		n = n / 2;
	}

	for(i = i - 1; i >= 0; i--)
		printf("%d",a[i]);

	putchar(',');
	putchar(' ');
	return 0;
}
