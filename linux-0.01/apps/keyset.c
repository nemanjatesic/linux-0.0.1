#include <unistd.h>
#include <string.h>
#define UTIL_IMPLEMENTATION
#include "utils.h"
#include <errno.h>

int main(char *args)
{
	int brojArg = get_argc(args);
	if (!(brojArg == 1 || brojArg == 2 || brojArg == 3)) {
		printstr("Broj argumenata nije odgovarajuci\n");
		_exit(1);
	}
	int mode;
	int br = 0;
	if (brojArg == 1){
		char *string;
		menjanjeEchoa(0);
		fgets(string,514,0);
		menjanjeEchoa(1);
		string[strlen(string)-1] = '\0';
		br = keyset(0,string);
		_exit(0);

	}	
	if (brojArg == 2){
		char *string = get_argv(args, 1);
		br = keyset(0,string);
	}
	
	if (brojArg == 3){
		mode = atoi(get_argv(args,1));
		char *string = get_argv(args,2);
		br = keyset(mode,string);
	}

	if (br < 0) {
		if (errno == ENOTPOWOFTWO)	printstr("Pogresno unet argument, kljuc mora imati duzinu stepena broja 2.\n");
		if (errno == ERANGE && brojArg == 3) printstr("Uneli ste pogresne parametre, uneti 0 za globalni ili 1 za lokalni kljuc.\n");
		_exit(1);
	}

	_exit(0);
}
