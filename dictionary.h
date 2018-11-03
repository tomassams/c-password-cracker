extern int found;

int compare_hashes(char* in_hash, char* guess);
void* dict_mmap_thread_runner(void* arg);
int guess_from_dictionary(char* hash, int num_hashes);
