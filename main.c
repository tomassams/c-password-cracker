#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <crypt.h>

static const char PASSCHARS[] = "abcdefghikjlmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+\"#&/()=?!@$|[]|{}";
static const int PASSCHARS_SIZE = sizeof(PASSCHARS) -1; 
static const int MAX_LENGTH = 4;

int dictionaryGuess(char* hash) {

	FILE* dictionaryFile = fopen("./dictionary.txt","r");
	
	char salt[13] = "$1$abcefgh$"; // placeholder.. todo: should this be 13 or 12? works either way
	char dictionaryWord[60];
	
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
			fclose(dictionaryFile);
			return 1;
		}

	}

	// no hits, cleanup and return
	fclose(dictionaryFile);
	return -1;
}

// recursively iterate through all passchar combinations
int iterate(char* combination, int index, char* hash, int wordlength) {

char salt[13] = "$1$abcefgh$"; // placeholder.. todo: should this be 13 or 12? works either way

// extract the salt from the encrypted input hash
for(int i = 3; i < 11; i++) {
	salt[i] = hash[i];
}

	if(index < wordlength) {
		for(int i = 0; i < PASSCHARS_SIZE; i++) {
			combination[index] = PASSCHARS[i];		

			if(iterate(combination, index + 1, hash, wordlength) == 1) {
				return 1; // we're done, stop looking
			}
		}
	}
	else {
		for(int i = 0; i < PASSCHARS_SIZE; i++) {
			combination[index] = PASSCHARS[i];	
			printf("Trying %s\n", combination);
			char* encrypted=crypt(combination,salt);

			if(strncmp(hash, encrypted, sizeof(char)*34) == 0) {
				printf("Found! %s\n", combination);
				return 1; // you did it!
			}
		}
	}

	return 0; // not found :(

}

void guessAllCombinations(char* hash) {

	char combinations[MAX_LENGTH +1];

	for(int i = 0; i < MAX_LENGTH; i++) {
		memset(combinations, 0, MAX_LENGTH + 1);
		if(iterate(combinations, 0, hash, i) == 1) return;
	}

}


int main(int argc, char** argv) {
	
	char* hash = argv[1];

	// validate input
	if(argc!=2 || strlen(hash) != 34) {
		printf("Invalid argument. Exiting..\n");
		return -1;
	}
	
	guessAllCombinations(hash);

	//dictionaryGuess(hash);

	return 0;

}
