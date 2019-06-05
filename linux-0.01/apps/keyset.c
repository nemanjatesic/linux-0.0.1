#include <unistd.h>
#include <string.h>
#define UTIL_IMPLEMENTATION
#include "utils.h"
#include <errno.h>

int main(char *args)
{
	int brojArg = get_argc(args);
	if (brojArg != 2) {
		printstr("Broj argumenata nije odgovarajuci\n");
		_exit(1);
	}
	char *string = get_argv(args, 1);
	int br = keyset(string);
	if (br < 0) {
		if (errno == ENOTPOWOFTWO)	printstr("Pogresno unet argument, kljuc mora imati duzinu stepena broja 2.\n");
		_exit(1);
	}

	_exit(0);
}