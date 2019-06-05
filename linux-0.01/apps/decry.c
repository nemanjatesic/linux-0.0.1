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
	int br = decry(string);
	if (br < 0) {
		if (errno == ENOFILE) printstr("Fajl koji trazite ne postoji.\n");
		if (errno == EISDIR) printstr("Ne mozete dekriptovati direktorijum.\n");
		if (errno == EKEYNOTFOUND) printstr("Enkripcioni kljuc ne postoji.\n");
		if (errno == EAENCR) printstr("Ne mozete dekriptovati vec dekriptovani fajl.\n");
		if (errno == EPERM) printstr("Ne mozete da obrisete taj fajl. Operacija nije dozvoljena.\n");
		_exit(1);
	}

	_exit(0);
}