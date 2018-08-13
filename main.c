#include <stdio.h>
#include <stdx/string_lookup.h>
#include <stdx/time.h>
#include <stdlib.h>
#include <stdx/string_dics/string_dic_naive.h>

static char *read_contents(const char *path)
{
    fprintf(stderr, "reading '%s'...", path);

    FILE    *infile;
    char    *buffer;
    long    numbytes;

    infile = fopen(path, "r");

    if(infile == NULL)
        return NULL;

    fseek(infile, 0L, SEEK_END);
    numbytes = ftell(infile);

    fseek(infile, 0L, SEEK_SET);

    buffer = (char*)malloc(numbytes * sizeof(char) + 1);

    if(buffer == NULL)
        return NULL;

    fread(buffer, sizeof(char), numbytes, infile);
    fclose(infile);

    buffer[numbytes] = '\0';

    fprintf(stderr, "DONE\n");

    return buffer;
}

struct vector *to_string_list(const char *contents)
{
    fprintf(stderr, "converting to line list...");
    struct vector *vector = malloc(sizeof(struct vector));
    vector_create(vector, NULL, sizeof(char *), 15372804);
    char *begin, *end;
    begin = (char *) contents;
    for (end = (char *) contents; *end != '\0'; end++) {
        size_t len = (end - begin);
        if (unlikely(*end == '\n' && len > 0)) {
            char *string = malloc(len + 1);
            memcpy(string, begin, len);
            string[len] = '\0';

            vector_push(vector, &string, 1);
            begin = end + 1;

        }
       // fprintf(stderr, "%f done so far...\n", vector_len(vector) / 4561977.0f * 100);
    }
    fprintf(stderr, "DONE, %zu lines\n", vector_len(vector));
    return vector;
}

#define NUM_SAMPLES 3

int main()
{
    timestamp_t read_begin = time_current_time_ms();
    //char *contents = read_contents("/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-15pc-stringlist.txt");
    char *contents = read_contents("/home/pinnecke/datasets/yago1/stringlists/yago1-11pc-stringlist.txt");
    timestamp_t read_end = time_current_time_ms();
    fprintf(stderr, "... %fsec\n", (read_end - read_begin) / 1000.0f);

    timestamp_t convert_begin = time_current_time_ms();
    struct vector *lines = to_string_list(contents);
    timestamp_t convert_end = time_current_time_ms();
    fprintf(stderr, "... %fsec\n", (convert_end - convert_begin) / 1000.0f);
    fflush(stderr);



    struct string_dic dic;

    float created_agg = 0;
    float insert_agg  = 0;

    for (int sample = 0; sample < NUM_SAMPLES; sample++) {
        printf("*** %d of %d in progress ***\n", sample + 1, NUM_SAMPLES);

        timestamp_t create_begin = time_current_time_ms();
        string_dic_create_naive(&dic, 10646182, 10.0*vector_len(lines), 10, 8, NULL);
        timestamp_t create_end = time_current_time_ms();
        created_agg += (create_end-create_begin);

        string_id_t* ids; //, * ids_out;

        char** strings = (char**) vector_data(lines);
        size_t num_strings = vector_len(lines)-1;

        string_dic_reset_counters(&dic);
        timestamp_t inserted_begin = time_current_time_ms();
        string_dic_insert(&dic, &ids, strings, num_strings);
        timestamp_t inserted_end = time_current_time_ms();
        insert_agg += (inserted_end-inserted_begin);

        struct string_lookup_counters counters;
        string_dic_counters(&counters, &dic);

        printf("num_bucket_search_miss;%zu\n", counters.num_bucket_search_miss);
        printf("num_bucket_search_hit;%zu\n", counters.num_bucket_search_hit);
        printf("num_bucket_cache_search_miss;%zu\n", counters.num_bucket_cache_search_miss);
        printf("num_bucket_cache_search_hit;%zu\n", counters.num_bucket_cache_search_hit);

       // char **strings_ex = string_dic_extract(&dic, ids, num_strings);
        // for (size_t i = 0; i < num_strings; i++) {
        //    printf("[%zu] -> { '%s' }\n", ids[i], strings_ex[i]);
        // }

        string_dic_free(&dic, ids);


    }

    fprintf(stderr, "created: ... %fsec\n", created_agg / NUM_SAMPLES / 1000.0f);
    fprintf(stderr, "inserted: ... %fsec\n",insert_agg / NUM_SAMPLES /1000.0f);
    fprintf(stderr, "----------------------------------\nTOTAL.........: %fsec (incl. 2sec I/O)\n", (2 + (created_agg+insert_agg) / NUM_SAMPLES /1000.0f));

//
//    timestamp_t locate_begin = time_current_time_ms();
//    string_dic_locate_fast(&ids_out, &dic, strings, num_strings);
//    timestamp_t locate_end = time_current_time_ms();
//    fprintf(stderr, "locate: ... %fsec\n", (locate_end - locate_begin) / 1000.0f);
//
//    timestamp_t extracted_begin = time_current_time_ms();
//    string_dic_extract(&dic, ids_out, num_strings);
//    timestamp_t extracted_end = time_current_time_ms();
//    fprintf(stderr, "extracted: ... %fsec\n", (extracted_end - extracted_begin) / 1000.0f);

    fflush(stderr);


  //  string_dic_free(&dic, ids);
  //  string_dic_free(&dic, ids_out);

    free(contents);
    vector_drop(lines);
    free(lines);

    //free (strings);

    return 0;
}