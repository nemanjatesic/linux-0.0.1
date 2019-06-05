#include <unistd.h>
#include <string.h>
#define UTIL_IMPLEMENTATION
#include "utils.h"
#include <errno.h>

static uint64_t seed;

void srand(unsigned s) {
	seed = s - 1;
}

int rand(void) {
	seed = 6364136223846793005ULL * seed + 1;
	return seed >> 33;
}

int main(char *args)
{
	int offset = 48;
	int max = 122 - offset;

	int brojArg = get_argc(args);
	if (brojArg != 2) {
		printstr("Broj argumenata nije odgovarajuci\n");
		_exit(1);
	}
	char *string = get_argv(args, 1);

	int broj = atoi(string);
	time(NULL);
	srand(-errno);	

	char randomKey[17];
	int i;
	for (i = 0; i < 17; ++i) randomKey[i] = '\0';

	if (broj >= 1 && broj <= 3){
		int k,j;
		if (broj == 1) k = 4;
		if (broj == 2) k = 8;
		if (broj == 3) k = 16;

		for (j = 0 ; j < k ; j++){
			int r = rand() % (max + 1);
			char slovo = r + offset;
			randomKey[j] = slovo;
		}
		randomKey[k+1] = '\0';
	}else {
		printstr("Los level uneti lvl izmedju [1-3]\n");
		_exit(1);
	}

	printstr(randomKey);
	printstr("\n");
	//keyset(randomKey);
	_exit(0);
}