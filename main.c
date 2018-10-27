#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <crypt.h>

static const char PASSCHARS[] = "abcdefghikjlmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+\"#&/()=?!@$|[]|{}";
static const int  PASSCHARS_SIZE = sizeof(PASSCHARS) -1; // because 1 char is 1 byte
static const int  MAX_WORD_LENGTH = 4;

// helper
int compareHashes(char* inHash, char* guess) {

	char salt[13] = "$1$abcdefgh$"; // placeholder, we replace abcde... in the loop below
	for(int i = 3; i < 11; i++) {
		salt[i] = inHash[i];
	}

	char* outHash = crypt(guess, salt);

	return strncmp(inHash, outHash, sizeof(char)*34); // is 0 if they match
}

// dictionary search
int guessFromDictionary(char* hash) {

	FILE* dictionaryFile = fopen("./dictionary.txt","r");
	
	char dictionaryWord[60];

	while(fscanf(dictionaryFile, "%s", dictionaryWord) != EOF) {

		printf("Trying word... %s\n", dictionaryWord);

		if(compareHashes(hash, dictionaryWord) == 0) {
			printf("SUCCESS! Match found: %s\n", dictionaryWord);
			fclose(dictionaryFile);
			return 1;
		}

	}

	fclose(dictionaryFile);
	return -1;
}

// recursive call for bruteforce
int guess(char* combination, int index, char* hash, int wordlength) {

	if(index < wordlength) {
		for(int i = 0; i < PASSCHARS_SIZE; i++) {
			combination[index] = PASSCHARS[i];		

			if(guess(combination, index + 1, hash, wordlength) == 1) {
				return 1; // we're done, stop looking
			}
		}
	}
	else {
		for(int i = 0; i < PASSCHARS_SIZE; i++) {
			combination[index] = PASSCHARS[i];	
			printf("Trying %s\n", combination);

			if(compareHashes(hash, combination) == 0) {
				printf("SUCCESS! Match found: %s\n", combination);
				return 1; // you did it!
			}
		}
	}

	return 0; // not found :(

}

// bruteforce every character combination
void guessAllCombinations(char* hash) {

	char combinations[MAX_WORD_LENGTH +1];

	for(int i = 0; i < MAX_WORD_LENGTH; i++) {
		//memset(combinations, 0, MAX_WORD_LENGTH + 1);
		if(guess(combinations, 0, hash, i) == 1) return;
	}

}


int main(int argc, char** argv) {
	
	char* hash = argv[1];

	// validate input
	if(argc!=2 || strlen(hash) != 34) {
		printf("Invalid argument. Exiting..\n");
		return -1;
	}

	// start with dictionary
	guessFromDictionary(hash);
	
	// nothing? then try brute force
	//guessAllCombinations(hash);

	return 0;

}
