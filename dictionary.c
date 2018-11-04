#include "helper.h"
#include "dictionary.h"

#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static const char DICTIONARY_DIR_PATH[40] = "./dictionaries";

struct dict_struct 
{ 
	int thread_id;
	int read_from;
	int read_to;
	char* mmapped_str;
	char* hash;
};

void* dictionary_thread_runner(void* arg) 
{

	struct dict_struct* args = (struct dict_struct*) arg;

	char word[60];
	int word_char = 0;

	// make sure we dont start our checks in the middle of a word
	while(args->thread_id != 0 && args->mmapped_str[args->read_from] != '\n') 
	{
		args->read_from--;
	}

	for(int i = args->read_from; i < args->read_to; i++) 
	{
		if(found == 1) pthread_exit(0); 

		word[word_char] = args->mmapped_str[i];

		if(args->mmapped_str[i] == '\n') 
		{	
			word[word_char] = '\0';
			word_char = 0;

			int guess = compare_hashes(args->hash, word);
			if(guess == 0) 
			{ 
				pthread_exit(0);
			}
		} 
		else 
		{
			word_char++;
		}
	}
	pthread_exit(0);
}

void dictionary_thread_starter(char* hash, int num_threads, char* file_name)
{
		
	printf("Attempting dictionary search with file %s...\n", file_name);

	char* file_path = calloc(strlen(DICTIONARY_DIR_PATH) + strlen(file_name) + 1, sizeof(char));
	sprintf(file_path, "%s/%s", DICTIONARY_DIR_PATH, file_name);

	int file_descriptor = open(file_path, O_RDONLY, S_IRUSR | S_IWUSR);
	struct stat file_info;
	
	if(fstat(file_descriptor, &file_info) == -1) 
	{
		printf("Error: Couldnt get file size. Aborting..\n");
		exit(0);
	}

	char* file_in_memory = mmap(NULL, file_info.st_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);

	struct dict_struct args[num_threads];
	pthread_t pthread_ids[num_threads];

	int chunk = (file_info.st_size / num_threads);

	for(int i = 0; i < num_threads; i++) 
	{ 
		args[i].thread_id = i;
		args[i].hash = hash;
		args[i].mmapped_str = file_in_memory;
		args[i].read_from = chunk * i; 

		if(i == num_threads - 1) 
		{
			args[i].read_to = ((chunk * (i+1)) + file_info.st_size % num_threads);
		}
		else 
		{
			args[i].read_to = chunk * (i+1);
		}

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&pthread_ids[i], &attr, dictionary_thread_runner, &args[i]);
	}

	for(int i = 0; i < num_threads; i++) 
	{
		pthread_join(pthread_ids[i], NULL);
	}

	munmap(file_in_memory, file_info.st_size);
	close(file_descriptor);
	free(file_path);
}

int guess_from_dictionary(char* hash, int num_threads) 
{
	
	printf("Starting dictionary search. Scanning dictionary file folder...\n");

	DIR *dictionary_dir;
    struct dirent* dir_entry;

    dictionary_dir = opendir(DICTIONARY_DIR_PATH);
	if(dictionary_dir == NULL) 
	{
		printf("Couldn't open dictionary file directory (%s). Aborting..\n", DICTIONARY_DIR_PATH);
		exit(0);
	}
	
	while ((dir_entry = readdir(dictionary_dir)) != NULL)
	{
		// skip current/previous folder
		if (!strcmp (dir_entry->d_name, "."))
			continue;
		if (!strcmp (dir_entry->d_name, ".."))    
			continue;	

		dictionary_thread_starter(hash, num_threads, dir_entry->d_name);
	}

	closedir(dictionary_dir);

	if(found == 1) return 0;

	return -1;
}