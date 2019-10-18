# Revision Control

## Commit Hash Computations

### Initial Commit Hash

For new records

```
bool global_id_create(global_id_t *out)
{
        static bool process_init;
        static u64 process_local_id;
        static u8 process_magic;
        static u64 process_counter;

        static u8 global_build_date_bit;
        static u8 global_build_path_bit;

        if (!process_init) {
                srand(time(NULL));
                process_magic = rand();
                process_init = true;
                process_counter = rand();

                const char *file = __FILE__;
                const char *time = __TIME__;

                global_build_path_bit = HASH_BERNSTEIN(strlen(file), file) % 2;
                global_build_date_bit = HASH_BERNSTEIN(strlen(time), time) % 2;
        }

        if (!thread_local_init) {
                thread_local_counter = rand();
                thread_local_counter_limit = thread_local_counter++;
                thread_local_id = (u64) pthread_self();
                process_local_id = getpid();
                thread_local_magic = rand();
                thread_local_init = true;
        }

        bool capacity_left = (thread_local_counter != thread_local_counter_limit);
        error_print_if(!capacity_left, ERR_THREADOOOBJIDS)
        if (likely(capacity_left)) {
                union global_id internal =
                        {.global_wallclock  = time_now_wallclock(), 
                         .global_build_date = global_build_date_bit, 
                         .global_build_path = global_build_path_bit, 
                         .process_id        = process_local_id, 
                         .process_magic     = process_magic, 
                         .process_counter   = process_counter++, 
                         .thread_id         = (u64) thread_local_id, 
                         .thread_magic      = thread_local_magic, 
                         .thread_counter    = thread_local_counter++, 
                         .call_random       = rand()};
                *out = internal.value;
        } else {
                *out = 0;
        }
        return capacity_left;
}
``` 

### Revision Commit Hash

Commit hash for revised record `R'` is _FNV_ hash value of raw data stream of original record `R`.

```
uint64_t hash = (uint64_t) 2166136261;
for (size_t i = 0; i < key_size; i++){
   hash = (hash * 16777619) ^ ((unsigned char* )key)[i];
}
```

## Stringified Commit Hash

The base 16 (hex) value string of a commit hash value with exactly 16 letters without any leading prefix.

Example base 10 commit hash `2072006001577230657` is printed to string as `1cc13e7b007d0141`.

## Initial Commit Hash



