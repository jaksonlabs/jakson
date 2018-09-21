#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "src/strdic_sync.h"
#include "src/strdic_async.h"
#include "src/bitmap.h"
#include "src/slicelist.h"
#include "src/chunkreader.h"
#include "src/time.h"
//#src <ng5/roadfire/roadfire.h>

#define NUM_SAMPLES 1
#define NTHREADS    (72 * 2)

/*void roadfire_test() {
    struct storage_engine engine;
    storage_engine_roadfire_create(&engine, NULL, NULL);
}*/

#define NUM_SLICE_INSERT 100000

void experiments_hashing()
{
    printf("chunk_num;sample;num_buckets;time_created_sec;time_inserted_sec;time_bulk_sum_created_inserted;num_strings_chunk;num_strings_total;num_distinct_strings\n");

    //const char* path = "/Volumes/PINNECKE EXT/science/cleaned_datasets/dbpedia-cleaned.txt";
    //const char* path = "/home/pinnecke/datasets/yago1/stringlists/yago1-15pc-stringlist.txt";
    const char* path = "/home/pinnecke/mnt/datasets/mag-cleaned.txt";
    //const char* path = "/Users/marcus/temp/file.txt";


    struct Dictionary dic;

    for (size_t num_buckets = 50000; num_buckets<=50000; num_buckets += 50000) {
        for (int sample = 0; sample<NUM_SAMPLES; sample++) {

            ChunkReader reader;
            ChunkReaderCreate(&reader, NULL, path, 5 * 1024 * 1024 * 1204);

            float created_duration = 0;
            float insert_duration = 0;

            fprintf(stderr, "*** %d of %d in progress ***\n", sample+1, NUM_SAMPLES);

            fprintf(stderr, "create..\n");

            timestamp_t create_begin = time_current_time_ms();
            string_dic_create_async(&dic, 3720000, num_buckets, 3720000, NTHREADS,
                    NULL);                         // <--------------------------------------------
            timestamp_t create_end = time_current_time_ms();
            created_duration = (create_end-create_begin)/1000.0f;

            timestamp_t next_begin = time_current_time_ms();
            Vector ofType(char *) *vector;

            size_t chunk_num = 0;
            size_t total_num = 0;

            while((vector = ChunkReaderNext(&reader))) {
                timestamp_t next_end = time_current_time_ms();
                fprintf(stderr, "got next %zu lines in %f sec\n", vector!=NULL ? ng5_vector_len(vector) : 0,
                        (next_end-next_begin)/1000.0f);

                StringId* ids = NULL, * ids_out;
                UNUSED(ids_out);

                char** strings = (char**) ng5_vector_data(vector);
                size_t num_strings = ng5_vector_len(vector)-1;

                fprintf(stderr, "insert..\n");

                string_dic_reset_counters(&dic);
                timestamp_t inserted_begin = time_current_time_ms();
                string_dic_insert(&dic, &ids, strings, num_strings, 0);
                timestamp_t inserted_end = time_current_time_ms();
                insert_duration = (inserted_end-inserted_begin)/1000.0f;

//                fprintf(stderr, "locate..\n");
//
//                string_dic_locate_fast(&ids_out, &dic, strings, num_strings);
//                for (size_t i = 0; i<num_strings; i++) {
//                    StringId id_created = ids[i];
//                    StringId id_located = ids_out[i];
//                    //debug("check", "[%s] -> %zu", strings[i], id_located);
//                    PANIC_IF_WARGS(id_created!=id_located,
//                            "mapping broken for string [%s] id '%zu': expected %zu, is %zu",
//                            strings[i], i, id_created, id_located);
//                    assert(id_created==id_located);
//                }
//
//                fprintf(stderr, "extract..\n");
//
//                char** extracted_strings = string_dic_extract(&dic, ids, num_strings);
//                for (size_t i = 0; i<num_strings; i++) {
//                    char* extracted = extracted_strings[i];
//                    char* given = strings[i];
//                    PANIC_IF(strcmp(extracted, given)!=0, "extraction broken");
//                    assert(strcmp(extracted, given)==0);
//                    //     debug("extracted id=%zu -> string [%s]\n", ids[i], extracted);
//                }
//
//                fprintf(stderr, "remove..\n");

                //  string_dic_remove(&dic, ids, num_strings);

                //struct string_lookup_counters counters;
                //string_dic_counters(&counters, &dic);

                size_t num_distinct;
                string_dic_num_distinct_values(&num_distinct, &dic);

                total_num +=  ng5_vector_len(vector);

                printf("%zu;%d;%zu;%f;%f;%f;%zu;%zu;%zu\n", chunk_num++, sample, num_buckets, created_duration,
                        insert_duration,
                        (created_duration+insert_duration), ng5_vector_len(vector), total_num, num_distinct);

                string_dic_free(&dic, ids);
                //string_dic_free(&dic, extracted_strings);
                //string_dic_free(&dic, ids_out);

                for (size_t i = 0; i<ng5_vector_len(vector); i++) {
                    char* string = *ng5_vector_get(vector, i, char *);
                    free(string);
                }
                VectorDrop(vector);
                free(vector);

            }

            fflush(stderr);
            fflush(stdout);

            ChunkReaderDrop(&reader);
            string_dic_drop(&dic);

        }
        exit(0);
    }
}

int main()
{
//roadfire_test();
    experiments_hashing();

    return 0;
}