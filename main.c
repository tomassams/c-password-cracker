#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dictionary.h"
#include "brute_force.h"

int main(int argc, char** argv) 
{	
	// validate input
	char* hash = argv[1];
	if(argc <= 1 || argc > 3 || strlen(hash) != 34) 
	{
		printf("Usage: %s <hash> <num_threads>\n", argv[0]);
		return -1;
	}

	int num_threads = 1;
	if(argv[2] != NULL && atoi(argv[2]) != 0) {
		num_threads = atoi(argv[2]);
	}

	printf("Searching for hash: %s\n", hash);

	if(guess_from_dictionary(hash, num_threads) == 1)
		return 1;

	printf("Dictionary search returned no hits...\n");

	if(guess_all_combinations(hash, num_threads) == 1)
		return 1;

	printf("No matches found!\n");
	return 0;
}