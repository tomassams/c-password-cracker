#include "helper.h"
#include "brute_force.h"

#include <pthread.h>
#include <stdio.h>

static const char PASSCHARS[] = "abcdefghikjlmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+\"#&/()=?!@$|[]|{}";
static const int  PASSCHARS_SIZE = sizeof(PASSCHARS) -1; // because 1 char is 1 byte
static const int  MAX_WORD_LENGTH = 4; 

struct brute_struct {
	int thread_id;
	char* hash;
	int start;
	int end;
};

int brute_recursive(char* word, char* hash, int start, int end, int c_index, int w_length, int t_id) {

	for(int i = 0; i < PASSCHARS_SIZE; i++) {

		word[c_index] = PASSCHARS[i];

		printf("Thread %d : Trying word... %s\n", t_id, word);

		if(compare_hashes(hash, word) == 0) { // base case, we're done
			return 0;
		} 
		else if(c_index < w_length) { // self call but only until reach word length
			int guess = brute_recursive(word, hash, start, end, c_index + 1, w_length, t_id);

			if(guess == 0) return 0; // we're done
		}

	}

	return -1;
}

void* brute_runner(void* arg) {
	struct brute_struct* args = (struct brute_struct*) arg;

	char word[MAX_WORD_LENGTH+1];

	for(int i = 0; i < MAX_WORD_LENGTH; i++) {
		int guess = brute_recursive(word, args->hash, args->start, args->end, 0, i, args->thread_id);
		if(guess == 0 || found == 1) pthread_exit(0);
	}
	pthread_exit(0);
	
}

int guess_all_combinations(char* hash, int num_threads) {

	pthread_t pthread_ids[num_threads];
	struct brute_struct args[num_threads];
	int chunk = (PASSCHARS_SIZE / num_threads);

	for(int i = 0; i < num_threads; i++) {
		args[i].thread_id = i;
		args[i].hash = hash;
		args[i].start = chunk * i;

		if(i == num_threads - 1) {
			args[i].end = ((chunk * (i+1)) + PASSCHARS_SIZE % num_threads);
		}
		else {
			args[i].end = chunk * (i+1);
		}
		
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&pthread_ids[i], &attr, brute_runner, &args[i]);
	}
	for(int i = 0; i < num_threads; i++) {
		pthread_join(pthread_ids[i], NULL);
	}
	return -1; // no hits

}

