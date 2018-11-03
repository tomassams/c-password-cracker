extern int found;

int brute_recursive(char* word, char* hash, int start, int end, int c_index, int w_length, int t_id);
void* brute_runner(void* arg);
int guess_all_combinations(char* hash, int num_threads);
