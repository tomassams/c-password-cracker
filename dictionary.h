extern int found;

void* dictionary_thread_runner(void* arg);
void  dictionary_thread_starter(char* hash, int num_threads, char* file_name);
int   guess_from_dictionary(char* hash, int num_threads);