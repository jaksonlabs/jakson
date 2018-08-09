#include <stdio.h>
#include <stdx/string_lookup.h>
#include <stdx/time.h>
#include <stdx/string_lookups/simple_bsearch.h>
#include <stdlib.h>
#include <stdx/string_dics/string_dic_naive.h>

#define NUM_STRINGS 1000

int main()
{
    struct string_dic dic;
    string_dic_create_naive(&dic, 1000, 1, NUM_STRINGS, 8, NULL);

    string_id_t *ids;

    char** strings = malloc(NUM_STRINGS * sizeof(char*));

    for (size_t i = 0; i < NUM_STRINGS; i++) {
        char buffer[129];
        sprintf(buffer, "string-%zu", i);
        strings[i] = strdup(buffer);
    }

    string_dic_insert(&dic, &ids, strings, NUM_STRINGS);

    string_dic_locate_fast(&ids, &dic, strings, NUM_STRINGS);

    char **extracted_strings = string_dic_extract(&dic, ids, NUM_STRINGS);

    for (size_t i = 0; i < NUM_STRINGS; i++) {
        assert(strcmp(extracted_strings[i], strings[i]) == 0);
        printf("id %zu -> '%s'\n", ids[i], extracted_strings[i]);
    }

    for (size_t i = 0; i < NUM_STRINGS; i++) {
        free (strings[i]);
    }

    free (strings);

    return 0;
}