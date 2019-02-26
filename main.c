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
#define NTHREADS    (1)

/*void roadfire_test() {
    struct storage_engine engine;
    storage_engine_roadfire_create(&engine, NULL, NULL);
}*/

#define NUM_SLICE_INSERT 100000

void experiments_hashing()
{
    printf("chunk_num;sample;num_buckets;time_created_sec;time_inserted_sec;time_bulk_sum_created_inserted;num_strings_chunk;num_strings_total;num_distinct_strings\n");

    // const char* path = "./benches/data/test.txt";
    //const char* path = "./benches/yago1-99pc-stringlist-cleaned.txt";
     const char* path = "./benches/yago1-short.txt";
    // const char* path = "./benches/tpch-sf10-cleaned.txt.list";
    //const char* path = "/home/pinnecke/datasets/yago1/stringlists/yago1-15pc-stringlist.txt";
    //const char* path = "/home/pinnecke/mnt/datasets/mag-cleaned.txt";
    //const char* path = "/Users/marcus/temp/file.txt";


    StringDictionary dic;

    for (size_t num_buckets = 50000; num_buckets<=50000; num_buckets += 50000) {
        for (int sample = 0; sample<NUM_SAMPLES; sample++) {

            ChunkReader reader;

            /**
             * Here occurs undefined behaviour on different systems. Problem is,
             * that on Linux the compiler treats the result of the calculation as int 
             * before casting it to size_t in the function call, but the result is too big
             * for int. Therefore, an explicit size_t cast is required to build
             * without werror
             **/
            ChunkReaderCreate(&reader, NULL, path, (size_t)5 * 1024 * 1024 * 1204);

            float created_duration = 0;
            float insert_duration = 0;

            fprintf(stderr, "*** %d of %d in progress ***\n", sample+1, NUM_SAMPLES);

            fprintf(stderr, "create..\n");

            Timestamp create_begin = TimeCurrentSystemTime();
            StringDictionaryCreateAsync(&dic, 3720000, num_buckets, 3720000, NTHREADS,
                                        NULL);                         // <--------------------------------------------
            Timestamp create_end = TimeCurrentSystemTime();
            created_duration = (create_end-create_begin)/1000.0f;

            Timestamp next_begin = TimeCurrentSystemTime();
            Vector ofType(char *) *vector;

            size_t chunk_num = 0;
            size_t total_num = 0;

            while((vector = ChunkReaderNext(&reader))) {
                Timestamp next_end = TimeCurrentSystemTime();
                fprintf(stderr, "got next %zu lines in %f sec\n", vector!=NULL ? VectorLength(vector) : 0,
                        (next_end-next_begin)/1000.0f);

                StringId* ids = NULL, * ids_out;
                UNUSED(ids_out);

                char** strings = (char**) VectorData(vector);
                size_t num_strings = VectorLength(vector)-1;

                fprintf(stderr, "insert..\n");

                StringDictionaryResetCounters(&dic);
                Timestamp inserted_begin = TimeCurrentSystemTime();
                StringDictionaryInsert(&dic, &ids, strings, num_strings, 0);
                Timestamp inserted_end = TimeCurrentSystemTime();
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
                StringDictionaryNumDistinct(&num_distinct, &dic);

                total_num += VectorLength(vector);

                printf("%zu;%d;%zu;%f;%f;%f;%zu;%zu;%zu\n", chunk_num++, sample, num_buckets, created_duration,
                        insert_duration,
                        (created_duration+insert_duration), VectorLength(vector), total_num, num_distinct);

                StringDictionaryFree(&dic, ids);
                //string_dic_free(&dic, extracted_strings);
                //string_dic_free(&dic, ids_out);

                for (size_t i = 0; i< VectorLength(vector); i++) {
                    char* string = *VECTOR_GET(vector, i, char *);
                    free(string);
                }
                VectorDrop(vector);
                free(vector);

            }

            fflush(stderr);
            fflush(stdout);

            ChunkReaderDrop(&reader);
            StringDictionaryDrop(&dic);

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