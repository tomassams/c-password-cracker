#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <crypt.h>

static const char PASSCHARS[] = "abcdefghikjlmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+\"#&/()=?!@$|[]|{}";
static const int  PASSCHARS_SIZE = sizeof(PASSCHARS) -1; // because 1 char is 1 byte
static const int  MAX_WORD_LENGTH = 4;

// helper
int compare_hashes(char* in_hash, char* guess) {

	char salt[13] = "$1$abcdefgh$"; // placeholder, we replace abcde... in the loop below

	for(int i = 3; i < 11; i++) {
		salt[i] = in_hash[i];
	}

	char* out_hash = crypt(guess, salt);

	return strncmp(in_hash, out_hash, sizeof(char)*34); // is 0 if they match
}

// dictionary search
int guess_from_dictionary(char* hash) {

	FILE* dictionary_file = fopen("./dictionary.txt","r");
	
	char word[60];

	while(fscanf(dictionary_file, "%s", word) != EOF) {
		printf("1Trying word... %s\n", word);
 
		if(compare_hashes(hash, word) == 0) {
			printf("SUCCESS! Match found: %s\n", word);
			fclose(dictionary_file);
			return 0;
		}

	}

	fclose(dictionary_file);
	return -1;
}

// recursive call for bruteforce
int guess(char* word, int char_index, char* hash, int word_length) {

	if(char_index < word_length) {
		for(int i = 0; i < PASSCHARS_SIZE; i++) {
			word[char_index] = PASSCHARS[i];		

			if(guess(word, char_index + 1, hash, word_length) == 0) {
				return 0; // we're done, stop looking
			}
		}
	}
	else {
		for(int i = 0; i < PASSCHARS_SIZE; i++) {
			word[char_index] = PASSCHARS[i];	
			printf("2Trying word... %s\n", word);

			if(compare_hashes(hash, word) == 0) {
				printf("SUCCESS! Match found: %s\n", word);
				return 0;
			}
		}
	}

	return -1; // not found :(

}

// bruteforce every character combination
int guess_all_combinations(char* hash) {

	char combinations[MAX_WORD_LENGTH +1];

	for(int i = 0; i < MAX_WORD_LENGTH; i++) {
		//memset(combinations, 0, MAX_WORD_LENGTH + 1);
		if(guess(combinations, 0, hash, i) == 0) return 0; // match found
	}

	return -1; // no hits

}

int guess_with_iteration(char* hash) {
	// TODO. Maybe. I've read iteration performs better?
	return -1;
}


int main(int argc, char** argv) {
	
	char* hash = argv[1];

	// validate input
	if(argc!=2 || strlen(hash) != 34) {
		printf("Invalid argument. Exiting..\n");
		return -1;
	}

	// start with dictionary
	if(guess_from_dictionary(hash) == 0)
		return 0;
	
	// nothing? then try brute force
	if(guess_all_combinations(hash) == 0)
		return 0;

	return -1;

}
