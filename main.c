#include <stdio.h>
#include <stdx/string_lookup.h>
#include <stdx/time.h>
#include <stdx/string_lookups/simple_bsearch.h>
#include <stdlib.h>

static const size_t MAX_SIZE_B = 10 * 1024 * 1024; /* 10 MB > L3 Cache */
static const size_t STEPS = 10000;
static const size_t STEP_SIZE = MAX_SIZE_B / STEPS;

int main()
{
    printf("NUM_PAIRS;SAMPLE;SELECTIVITY;CREATE_MSPP;PUT_MSPP;GET_MSPP;DATA_SIZE\n");

    size_t bucket_size_bsearch = 32;
    size_t max_bytes = MAX_SIZE_B / bucket_size_bsearch;

    for (size_t CURRENT_BYTES = STEP_SIZE; CURRENT_BYTES < max_bytes; CURRENT_BYTES += STEP_SIZE) {
        for (size_t sample = 0; sample < 5; sample++) {
            for (int i = 5; i <= 100; i += 5) {

                timestamp_t create_begin, create_end;
                timestamp_t put_begin, put_end;
                timestamp_t get_begin, get_end;

                struct string_lookup map;

                create_begin = time_current_time_ms();
                string_hashtable_create_besearch(&map, NULL, 1, CURRENT_BYTES, 1.7f);
                create_end = time_current_time_ms();

                char** keys = malloc(CURRENT_BYTES*sizeof(char*));
                string_id_t* values = malloc(CURRENT_BYTES*sizeof(uint64_t));
                for (size_t i = 0; i<CURRENT_BYTES; i++) {
                    char buffer[129];
                    sprintf(buffer, "string-%zu", i);
                    keys[i] = strdup(buffer);
                    values[i] = i;
                }

                fprintf(stderr, "[INFO] PUT %zu KiB (%zu strings)...\n", CURRENT_BYTES/1024, CURRENT_BYTES);
                put_begin = time_current_time_ms();
                string_lookup_put_safe(&map, keys, values, CURRENT_BYTES);
                put_end = time_current_time_ms();

                string_id_t* out_values;
                bool* out_mask;
                size_t num_not_found;

                get_begin = time_current_time_ms();
                fprintf(stderr, "[INFO] GET %zu KiB...\n", (size_t)(CURRENT_BYTES * (i / 100.0f)));
                string_lookup_get_safe(&out_values, &out_mask, &num_not_found, &map, keys, CURRENT_BYTES*(i/100.0f));
                get_end = time_current_time_ms();

                printf("%zu;%zu;%f;%f;%f;%f;%zu\n", CURRENT_BYTES, sample, i / 100.0f,
                        (create_end-create_begin)/(float) CURRENT_BYTES,
                        (put_end-put_begin)/(float) CURRENT_BYTES,
                        (get_end-get_begin)/(float) CURRENT_BYTES,
                        CURRENT_BYTES);

                string_lookup_free(out_values, &map);
                string_lookup_free(out_mask, &map);
                string_lookup_drop(&map);
            }
        }
    }

    return 0;
}