#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "token.h"


#define DEBUG_TOKENIZER 0x1
#define DEBUG_IDENTIFIER 0x2

struct option const longopts[] =
{
	{"debug",       no_argument, 0, 'd'},
	{"help",	    no_argument, 0, 'h'},
	{"debug-token", no_argument, 0, 't'},

	{0, 0, 0, 0}
};


static FILE *open_file (const char *file_name);
static void build_option (int argc, char *argv[]);
static void help ();

static char *file_in = "d.c";
static int debug = 0;
static Ident *ident_head;
static Token *token_head;


int main (int argc, char *argv[])
{
	FILE *in;

	build_option(argc, argv);
	in = open_file(argv[argc - 1]);
	tokenize(in, &token_head, &ident_head);
	delete_spaces(token_head);
	if (debug & DEBUG_TOKENIZER)
        print_token_head(token_head);

	identifier(&token_head, &ident_head);
	if (debug & DEBUG_IDENTIFIER)
		print_ident_head(ident_head);

	//priority(token_head);
//	print_token_head(token_head);
	parser(token_head);
}


static void build_option (int argc, char *argv[])
{

	int opt_i = 0;
	int idk; /* idk stays for "I Dont't Know" :D */

	while ((idk = getopt_long(argc, argv, "dhtf:", longopts, &opt_i)) != -1) {
		switch (idk) {
		case 'h': help();    exit(0);
		case 'd': debug = UINT_MAX; break;
		case 't': debug |= DEBUG_TOKENIZER; break;
		case 'i': debug |= DEBUG_IDENTIFIER; break;
		case 'f':
			file_in = realloc(file_in, strlen(optarg) + 1);
			strcpy(file_in, optarg);
			break;

		case ':':
			fprintf(stderr,
				"%s: option %c requires an argument.\n",
				argv[0], optopt);
			exit(1);

		case '?':
		default:
			fprintf(stderr,
			        "%s: option %c is invalid.\n",
			        argv[0], optopt);
			exit(1);
		};
	}
}

static void help ()
{
	printf("Get the fuck out here, this is shit ass pet fuckers");
	//printf("Are you ready to fuck with the source? Well... if not, don't use"
	  //    " this compiler\n");
}

static FILE *open_file (const char *file_name)
{
	FILE *in;

	if (!file_in) {
		file_in = realloc(file_in, strlen(file_name + 1));
		strcpy(file_in, file_name);
	}

	if (!(in = fopen(file_in, "r")))
		fatal("No source code in input");

	return in;
}
