#include <stdio.h>
#include <stdlib.h>

static size_t __hash (char *str);

size_t hash (char *s)
{
	/*
	char *p = s;
	size_t hashed = 0;
	const size_t key = 0xa40412dff;

	while (*p) {
		hashed += (*p) * key;
		p++;
	}

	return hashed;
	*/
	return __hash(s);
}


static size_t __hash (char *str)
{
	size_t hval = 0;
	unsigned char *s = (unsigned char *)str;	/* unsigned string */

	/*
	* FNV-1a hash each octet of the string
	*/
	while (*s) {
		hval ^= (size_t)*s++;

	/* multiply by the 64 bit FNV magic prime mod 2^64 */
		hval += (hval << 1) + (hval << 4) + (hval << 5) +
			(hval << 7) + (hval << 8) + (hval << 40);
	}
	return hval;
}

