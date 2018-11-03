#include "helper.h"
#include "dictionary.h"

#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

struct d_mmap_struct { 
	int thread_id;
	int read_from;
	int read_to;
	char* mmapped_str;
	char* hash;
};

void* dict_mmap_thread_runner(void* arg) {

	struct d_mmap_struct* args = (struct d_mmap_struct*) arg;

	char word[60];
	int word_char = 0;

	// make sure we dont start our checks in the middle of a word
	while(args->thread_id != 0 && args->mmapped_str[args->read_from] != '\n') {
		args->read_from--;
	}

	for(int i = args->read_from; i < args->read_to; i++) {
		if(found == 1) pthread_exit(&found); 

		word[word_char] = args->mmapped_str[i];

		if(args->mmapped_str[i] == '\n') {	
			word[word_char] = '\0';
			word_char = 0;

			printf("Thread %d: Trying word.. %s\n", args->thread_id, word);

			int guess = compare_hashes(args->hash, word);
			if(guess == 0) { 
				pthread_exit(&found);
			}
		} 
		else {
			word_char++;
		}
	}
	pthread_exit(&found);
}

int guess_from_dictionary(char* hash, int num_threads) {

	int file_descriptor = open("./dicts/dictionary.txt", O_RDONLY, S_IRUSR | S_IWUSR);
	struct stat file_info;

	if(fstat(file_descriptor, &file_info) == -1) {
		printf("Error: Couldnt get file size. Aborting..\n");
		return -1;
	}

	char* file_in_memory = mmap(NULL, file_info.st_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);

	struct d_mmap_struct args[num_threads];
	pthread_t pthread_ids[num_threads];

	int chunk = (file_info.st_size / num_threads);

	for(int i = 0; i < num_threads; i++) { 
		args[i].thread_id = i;
		args[i].hash = hash;
		args[i].read_from = chunk * i; 
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

	// TODO: add some sort of feedback to let the user know it's still running..

	for(int i = 0; i < num_threads; i++) {
		pthread_join(pthread_ids[i], NULL);
	}

	munmap(file_in_memory, file_info.st_size);
	close(file_descriptor);
	
	if(found == 1) return 0;

	return -1;
}
