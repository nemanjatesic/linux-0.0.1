#include <unistd.h>
#include <string.h>
#define UTIL_IMPLEMENTATION
#include "utils.h"
#include <errno.h>

int main(char *args)
{
	int brojArg = get_argc(args);
	if (!(brojArg == 1 || brojArg == 2)) {
		printstr("Broj argumenata nije odgovarajuci\n");
		_exit(1);
	}
	int mode;
	int br;
	if (brojArg == 1){
		br = keyclear(0);
	}
	if (brojArg == 2){
		mode = atoi(get_argv(args,1));
		br = keyclear(mode);
	}
	if (br < 0) {
		if (errno == ERANGE && brojArg == 2) printstr("Uneli ste pogresne parametre, uneti 0 za globalni ili 1 za lokalni kljuc.\n");
		_exit(1);
	}

	_exit(0);
}