#define _GNU_SOURCE

#include <crypt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"

int found = 0;

int compare_hashes(char* in_hash, char* guess) 
{
	struct crypt_data data;
	data.initialized = 0;

	char salt[13] = "$1$abcdefgh$";
	for(int i = 3; i < 11; i++) 
	{
		salt[i] = in_hash[i];
	}

	char* out_hash = crypt_r(guess, salt, &data);
	
	if(strncmp(in_hash, out_hash, sizeof(char)*34) == 0) 
	{
		printf("SUCCESS! Password found: %s\n", guess);
		found = 1;
		return 0;
	}

	return -1;
}
