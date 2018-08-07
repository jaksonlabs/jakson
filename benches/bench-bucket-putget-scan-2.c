#include <stdio.h>
#include <stdx/string_hashtable.h>
#include <stdx/time.h>
#include <stdx/string_hashtables/simple_scan1.h>
#include <stdlib.h>
#include <stdx/string_hashtables/simple_scan2.h>

#define TYPE "SIMPLE_SCAN_2"
/* Cache sizes on server: L1/L2/L3 1024/16384/22528 */

static const size_t MAX_SIZE_B = (2 * 22528 * 1024) / 32; /* 2*L3 Cache in Byte where 32bit are one bucket entry */
static const size_t STEPS = 25;
static const size_t STEP_SIZE = MAX_SIZE_B / STEPS;

int main()
{
    printf("TYPE;NUM_PAIRS;SAMPLE;SELECTIVITY;CREATE_MSPP;PUT_MSPP;GET_MSPP;DATA_SIZE;REP;NTHREADS\n");

    for (size_t CURRENT_BYTES = 1024; CURRENT_BYTES < MAX_SIZE_B; CURRENT_BYTES += STEP_SIZE) {
        for (size_t sample = 0; sample < 5; sample++) {
            for (int i = 5; i <= 100; i += 5) {
                for (int rep = 0; rep <= 100; rep += 10) {

                    timestamp_t create_begin, create_end;
                    timestamp_t put_begin, put_end;
                    timestamp_t get_begin, get_end;

                    struct string_hashtable map;

                    create_begin = time_current_time_ms();
                    string_hashtable_create_scan2(&map, NULL, 1, CURRENT_BYTES, 1.7f);
                    create_end = time_current_time_ms();

                    char** keys = malloc(CURRENT_BYTES*sizeof(char*));
                    char** search_keys = malloc(CURRENT_BYTES*sizeof(char*));
                    uint64_t* values = malloc(CURRENT_BYTES*sizeof(uint64_t));

                    for (size_t i = 0; i<CURRENT_BYTES; i++) {
                        char buffer[129];
                        sprintf(buffer, "string-%zu", i);
                        keys[i] = strdup(buffer);
                        values[i] = i;
                    }

                    for (size_t i = 0; i<CURRENT_BYTES; ) {
                        int k = i;
                        for (int j = 0; j<=(rep/100.0f * CURRENT_BYTES) && i<CURRENT_BYTES; j++, i++) {
                            search_keys[i] = keys[k];
                        }
                    }

                    fprintf(stderr, "[INFO] PUT %zu KiB (%zu strings)...\n", CURRENT_BYTES/1024, CURRENT_BYTES);
                    put_begin = time_current_time_ms();
                    string_hashtable_put_test(&map, keys, values, CURRENT_BYTES);
                    put_end = time_current_time_ms();

                    uint64_t* out_values;
                    bool* out_mask;
                    size_t num_not_found;

                    get_begin = time_current_time_ms();
                    fprintf(stderr, "[INFO] GET %zu KiB...\n", (size_t) (CURRENT_BYTES/1024*(i/100.0f)));
                    string_id_map_get_test(&out_values, &out_mask, &num_not_found, &map, search_keys,
                            CURRENT_BYTES*(i/100.0f));
                    get_end = time_current_time_ms();

                    printf("%s;%zu;%zu;%f;%f;%f;%f;%zu;%d;0\n",
                            TYPE,
                            CURRENT_BYTES,
                            sample,
                            i/100.0f,
                            (create_end-create_begin)/(float) CURRENT_BYTES,
                            (put_end-put_begin)/(float) CURRENT_BYTES,
                            (get_end-get_begin)/(float) CURRENT_BYTES,
                            CURRENT_BYTES,
                            rep);
                    fflush(stdout);

                    string_id_map_free(out_values, &map);
                    string_id_map_free(out_mask, &map);
                    string_hashtable_drop(&map);
                    for (size_t i = 0; i<CURRENT_BYTES; i++) {
                        free (keys[i]);
                    }
                    free (keys);
                    free (search_keys);
                }
            }
        }
    }

    return 0;
}