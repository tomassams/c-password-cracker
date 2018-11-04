#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "helper.h"
#include "brute_force.h"

static const char PASSCHARS[] = "abcdefghikjlmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+\"#&/()=?!@$|[]|{}";
static const int  PASSCHARS_SIZE = sizeof(PASSCHARS) -1; // because 1 char is 1 byte
static const int  MAX_WORD_LENGTH = 4; 

struct brute_struct {
	char* hash;
	int start;
	int end;
};

int brute_recursive(char* word, char* hash, int c_index, int w_length) 
{
	
	for(int i = 0; i < PASSCHARS_SIZE; i++) 
	{
		if(found == 1)
			return 0;
		
		word[c_index] = PASSCHARS[i];

		if(compare_hashes(hash, word) == 0)
			return 0; 

		if(c_index < w_length)
		{
			int guess = brute_recursive(word, hash, c_index + 1, w_length);
			if(guess == 0) 
				return 0;
		}
	}

	return -1;
}

void* brute_thread_runner(void* arg) 
{
	struct brute_struct* args = (struct brute_struct*) arg;

	int i = 0;
	while(i < MAX_WORD_LENGTH && found != 1)
	{
		char* word = calloc(MAX_WORD_LENGTH + 1, sizeof(char));

		for(int j = args->start; j < args->end; j++) 
		{
			if(found == 1) 
				break;

			word[0] = PASSCHARS[j];

			if(compare_hashes(args->hash, word) == 0) 
				break;

			int guess = brute_recursive(word, args->hash, 1, i);
			if(guess == 0) 
				break;
		}	
		i++;
		free(word);
	}
	pthread_exit(0);
}

int guess_all_combinations(char* hash, int num_threads) 
{
	printf("Attempting to brute force.. This might take a while...\n");

	pthread_t pthread_ids[num_threads];
	struct brute_struct args[num_threads];
	int chunk = (PASSCHARS_SIZE / num_threads);

	for(int i = 0; i < num_threads; i++) 
	{
		args[i].hash = hash;
		args[i].start = chunk * i;

		if(i == num_threads - 1)
		{
			args[i].end = ((chunk * (i+1)) + PASSCHARS_SIZE % num_threads);
		}
		else 
		{
			args[i].end = chunk * (i+1);
		}
		
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&pthread_ids[i], &attr, brute_thread_runner, &args[i]);
	}

	for(int i = 0; i < num_threads; i++) 
	{
		pthread_join(pthread_ids[i], NULL);
	}

	if(found == 1) 
		return 0; 

	return -1;
}