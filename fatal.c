#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "token.h"

extern struct token_struct token_list[];
//Declared in 'parser.c'
extern unsigned int current_line;

void __fatal (const char *func, char *msg, ...)
{
	va_list arg;
	char *s;
	long l;
	int d;
	Token *t;
	Ident *i;

	va_start(arg, msg);

	printf("%s(): %d; ", func, current_line);
	for (; *msg; msg++) {
		if (*msg == '%') {
			msg++;

			switch(*msg) {
			case 's':
				s = va_arg(arg, char *);
				printf("%s", s);
				break;

			case 'l':
				l = va_arg(arg, long);
				printf("%ld", l);
				break;

			case 't':
				t = va_arg(arg, Token *);
				printf("'%s'", token_list[t->tok - 1].word);
				break;

			case 'i':
				i = va_arg(arg, Ident *);
				printf("'%s'", i->name);
				break;

			case 'd':
				d = va_arg(arg, int);
				printf("'%d'", d);
				break;

			default:
				printf("\nUnknown cast (%c)", *msg);
				exit(1);

			};
		} else {
			putc(*msg, stdout);
		}
	}
	putc('\n', stdout);
	exit(1);
}
