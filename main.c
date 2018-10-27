#include <stdio.h>
#include <unistd.h>
#include <crypt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char** argv) {
	
	FILE* dictionaryFile = fopen("dictionary.txt","r");

	char salt[13] = "$1$abcdefgh$"; // placeholder
	char dictionaryWord[60]; // lets assume the words in the dictionary arent longer than this..
	char* hash = argv[1];

	// validate input
	if(argc!=2 || strlen(hash) != 34) {
		printf("Invalid argument. Exiting..\n");
		return -1;
	}

	// extract the salt from the encrypted input hash
	for(int i = 3; i < 11; i++) {
		salt[i] = hash[i];
	}

	// open our dictionary file and compare
	while(fscanf(dictionaryFile, "%s", dictionaryWord) != EOF) {

		printf("Trying word... %s\n", dictionaryWord);
		char* encrypted=crypt(dictionaryWord,salt);

		if(strncmp(hash, encrypted, sizeof(char)*34) == 0) {
			printf("SUCCESS! Match found: %s\n", dictionaryWord);
			return 0;
		}

	}

	// cleanup
	fclose(dictionaryFile);
	//free(encrypted);
	return 0;

}
