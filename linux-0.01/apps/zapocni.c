#include <unistd.h>
#include <string.h>
#define UTIL_IMPLEMENTATION
#include "utils.h"
#include <errno.h>

int main(char *args)
{
	int brojArg = get_argc(args);
	if (brojArg > 2) {
		printstr("Broj argumenata nije odgovarajuci.\n");
		_exit(1);
	}
	char * str = "/root/.encryptedList";
	if (brojArg == 1){
		int br = zapocni(str,0);
		if (br < 0)
			_exit(1);
		_exit(0);
	}else if (brojArg == 2){
		char *string = get_argv(args, 1);
		if(strlen(string) == 4 && string[0] == 'h' && string[1] == 'e' && string[2] == 'l' && string[3] == 'p' && string[4] == '\0'){
			printstr("Argument moze biti :\n0 - inicijalizuje glavni node za enkripciju\n1 - inicijalizuje glavni node i brise sve sto je bilo u njemu\n2 - debug mode\n");
			_exit(0);
		}
		int broj = atoi(string);
		if (!(broj == 0 || broj == 1 || broj == 2)){
			printstr("Argument moze biti 0 ili 1.\n");
			_exit(1);
		}
		int br = zapocni(str,broj);
		if (br < 0){
			_exit(1);
		}
		_exit(0);
	}
	_exit(0);
}