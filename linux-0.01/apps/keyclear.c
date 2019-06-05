#include <unistd.h>
#include <string.h>
#define UTIL_IMPLEMENTATION
#include "utils.h"
#include <errno.h>

int main(char *args)
{
	int brojArg = get_argc(args);
	if (brojArg != 1) {
		printstr("Broj argumenata nije odgovarajuci\n");
		_exit(1);
	}
	int br = keyclear();
	
	if (br < 0) {
		_exit(1);
	}

	_exit(0);
}