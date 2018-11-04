extern int found;

int   brute_recursive(char* word, char* hash, int c_index, int w_length);
void* brute_thread_runner(void* arg);
int   guess_all_combinations(char* hash, int num_threads);