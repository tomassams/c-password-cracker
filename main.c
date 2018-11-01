#define _GNU_SOURCE

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

#include <sys/mman.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <crypt.h>
#include <pthread.h>

static const char PASSCHARS[] = "abcdefghikjlmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+\"#&/()=?!@$|[]|{}";
static const int  PASSCHARS_SIZE = sizeof(PASSCHARS) -1; // because 1 char is 1 byte
static const int  MAX_WORD_LENGTH = 4;

int found = 0;

// helper
int compare_hashes(char* in_hash, char* guess) {
	struct crypt_data data;
	data.initialized = 0;

	char salt[13] = "$1$abcdefgh$"; // placeholder, we replace abcde... in the loop below
	for(int i = 3; i < 11; i++) {
		salt[i] = in_hash[i];
	}

	char* out_hash = crypt_r(guess, salt, &data);

	return strncmp(in_hash, out_hash, sizeof(char)*34); // is 0 if they match
}

struct d_thr_struct {
	int thread_id;
	FILE* dictionary_file;
	char* hash;
};

void* dict_thread_runner(void* arg) {

	struct d_thr_struct* args = (struct d_thr_struct*) arg;

	char word[60];

	while(fscanf(args->dictionary_file, "%s", word) != EOF) {

		if(found == 1) pthread_exit(&found); // exit if match is found already

		printf("Thread %d: Trying word... %s\n",args->thread_id, word);
 
		if(compare_hashes(args->hash, word) == 0) {
			printf("Thread %d: SUCCESS! Match found: %s\n", args->thread_id, word);
			found = 1;
			pthread_exit(&found);
		}

	}

	pthread_exit(&found); // no hits
}

int guess_from_dictionary(char* hash, int num_threads) {

	FILE* dictionary_file = fopen("./dicts/dictionary.txt","r");
	if(dictionary_file == NULL) {
		printf("Error: coudln't read dictionary file. Aborting..\n");
		return -1;
	}

	struct d_thr_struct args[num_threads];
	pthread_t pthread_ids[num_threads];

	for(int i = 0; i < num_threads; i++) {
		args[i].thread_id = i;
		args[i].hash = hash;
		args[i].dictionary_file = dictionary_file;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&pthread_ids[i], &attr, dict_thread_runner, &args[i]);
	}

	for(int i = 0; i < num_threads; i++) {
		pthread_join(pthread_ids[i], NULL);
	}

	fclose(dictionary_file);

	if(found == 1) 
		return 0;

	return -1;
}

// recursive call for bruteforce
int recursive_guess(char* word, int char_index, char* hash, int word_length) {

	for(int i = 0; i < PASSCHARS_SIZE; i++) {
		word[char_index] = PASSCHARS[i];
		printf("3Trying word... %s\n", word);

		if(compare_hashes(hash, word) == 0) { // base case, we're done
			printf("SUCCESS! Match found: %s\n", word);
			return 0;
		} 
		else if(char_index < word_length) { // self call but only until reach word length
			int guess = recursive_guess(word, char_index + 1, hash, word_length);

			if(guess == 0) 
				return 0; // we're done
		}
	}

	return -1; // no hits

}

// bruteforce every character combination
int guess_all_combinations(char* hash, int num_threads) {

	char word[MAX_WORD_LENGTH +1];

	for(int i = 0; i < MAX_WORD_LENGTH; i++) {
		int guess = recursive_guess(word, 0, hash, i);
		
		if(guess == 0) return 0; // we're done
	}

	return -1; // no hits

}

struct d_mmap_struct {
	int thread_id;
	int read_from;
	int read_to;
	char* mmapped_str;
	char* hash;
};

void* dict_mmap_thread_runner(void* arg) {

	struct d_mmap_struct* args = (struct d_mmap_struct*) arg;

	char word[60] = { 0 };
	int word_char = 0;

	// make sure we dont start our thread in the middle of a word
	while(args->thread_id != 0 && args->mmapped_str[args->read_from] != '\n') {
		args->read_from--;
	}

	// go through char for char
	for(int i = args->read_from; i < (args->read_from + args->read_to); i++) {
		if(found == 1) pthread_exit(&found); 

		// map chars to word
		word[word_char] = args->mmapped_str[i];

		// stop mapping when we reach a linebreak
		if(args->mmapped_str[i] == '\n') {	
			word[word_char] = '\0';
			word_char = 0;

			//printf("Thread %d: Trying word.. %s\n", args->thread_id, word);

			int guess = compare_hashes(args->hash, word);
			if(guess == 0) { 
				printf("Success! found a match: %s\n", word); 
				found = 1;
				pthread_exit(&found);
			}
		} 
		else {
			word_char++;
		}
	}
	pthread_exit(&found);
}

int guess_from_dictionary_mmap(char* hash, int num_threads) {

	int file_descriptor = open("./dicts/dictionary.txt", O_RDONLY, S_IRUSR | S_IWUSR);
	struct stat file_info;

	if(fstat(file_descriptor, &file_info) == -1) {
		printf("Error: Couldnt get file size. Aborting..\n");
		return -1;
	}

	char* file_in_memory = mmap(NULL, file_info.st_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);

	struct d_mmap_struct args[num_threads];
	pthread_t pthread_ids[num_threads];

	//arg[i].end = (i == num_thread-1) ?((chunk*(i+1)) + st.st_size%num_thread) : chunk*(i+1);
	int chunk = (file_info.st_size / num_threads);

	// make 1 thread for test:
	for(int i = 0; i < num_threads; i++) { 
		args[i].thread_id = i;
		args[i].hash = hash;
		args[i].read_from = chunk * i; 
		//args[i].read_to = chunk;
		args[i].mmapped_str = file_in_memory;

		if(i == num_threads - 1) {
			args[i].read_to = ((chunk * (i+1)) + file_info.st_size % num_threads);
		}
		else {
			args[i].read_to = chunk * (i+1);
		}

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&pthread_ids[i], &attr, dict_mmap_thread_runner, &args[i]);
	}

	for(int i = 0; i < num_threads; i++) {
		pthread_join(pthread_ids[i], NULL);
	}

	//cleanup
	munmap(file_in_memory, file_info.st_size);
	close(file_descriptor);

	return -1;
}

int main(int argc, char** argv) {

	// validate input
	// TODO: implement dictionary_file_path as argument
	char* hash = argv[1];
	if(argc <= 1 || argc > 4 || strlen(hash) != 34) {
		printf("Usage: %s <hash> <num_threads> <dictionary_file_path>\n", argv[0]);
		return -1;
	}

	// num_threads is optional, default to 1
	int num_threads = 1;
	if(argv[2] != NULL && atoi(argv[2]) != 0) {
		num_threads = atoi(argv[2]);
	}

	// mmap dictionary guess
	guess_from_dictionary_mmap(hash, num_threads);
		return 0;

	// start guessing from dictionary
	if(guess_from_dictionary(hash, num_threads) == 0)
		return 0;
	
	// nothing? then try brute force
	if(guess_all_combinations(hash, num_threads) == 0)
		return 0;

	printf("No matches found!\n");
	return 0;

}
