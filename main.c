#include "src/bitmap.h"
#include "src/chunkreader.h"
#include "src/slicelist.h"
#include "src/strdic_async.h"
#include "src/strdic_sync.h"
#include "src/time.h"
#include "src/information.h"
#include "src/descent.h"
#include <slog.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
//#src <ng5/roadfire/roadfire.h>


#define NUM_SLICE_INSERT 100000
#define BATCH_SIZE (size_t)5 * 1024 * 1024

typedef struct {
  float created_duration;
  float insert_duration;
  float total_duration;
} stats;


void experiments_hashing() {
  slog_info(0, "chunk_num;sample;num_buckets;time_created_sec;time_inserted_"
               "sec;time_bulk_sum_created_inserted;num_strings_chunk;num_"
               "strings_total;num_distinct_strings");

  slog_info(0, "Reading in environment");

  FILE *statistics_file = fopen("statistics.csv", "w");
  stats statistics = {0.0, 0.0, 0.0};
  context system_context = {0, 0, 0, 0};

  fprintf(statistics_file, "%s, %s, %s, %s\n", "thread_num", "created_duration",
          "insert_duration", "total_time");

  const char *path = getenv("NG5_SET");
  if (path == NULL) {
    slog_panic(0, "No dataset defined; Define NG5_SET");
    exit(1);
  }

  const char *max_threads_string = getenv("NG5_THREADS");
  if (max_threads_string == NULL) {
    slog_panic(0, "No maximum threads defined; Define NG5_THREADS");
    exit(1);
  }
  const size_t max_threads = (size_t)atoi(max_threads_string);

  const char *samples_string = getenv("NG5_SAMPLES");
  if (samples_string == NULL) {
    slog_panic(0, "No sample number defined; Define NG5_SAMPLES");
  }
  const size_t samples = (size_t)atoi(samples_string);

  system_info(path, &system_context, BATCH_SIZE);
  DescentInit();
  DescentCalculate(system_context);
  StringDictionary dic;
  for (size_t num_threads = 1; num_threads <= max_threads; num_threads++) {
    slog_info(0, "%s %zu %s\n", "PERFORMING SAMPLE WITH", num_threads,
              "THREADS");
    for (size_t num_buckets = 50000; num_buckets <= 50000;
         num_buckets += 50000) {
      for (size_t sample = 1; sample <= samples; sample++) {

        slog_info(0, "PERFORMING SAMPLE %zu", sample);

        ChunkReader reader;

        /**
         * Here occurs undefined behaviour on different systems. Problem is,
         * that on Linux the compiler treats the result of the calculation as
         *int before casting it to size_t in the function call, but the result
         *is too big for int. Therefore, an explicit size_t cast is required to
         *build without werror
         **/
        ChunkReaderCreate(&reader, NULL, path, BATCH_SIZE);

        float created_duration = 0;
        float insert_duration = 0;

        slog_info(0, "*** %d of %d in progress ***", sample, samples);
        slog_info(0, "create..");

        Timestamp create_begin = TimeCurrentSystemTime();
        StringDictionaryCreateAsync(&dic, 3720000, num_buckets, 3720000,
                                    num_threads, NULL);
        Timestamp create_end = TimeCurrentSystemTime();
        created_duration = (create_end - create_begin) / 1000.0f;

        Timestamp next_begin = TimeCurrentSystemTime();
        Vector ofType(char *) * vector;

        size_t chunk_num = 0;
        size_t total_num = 0;

        size_t iteration = 0;

        while ((vector = ChunkReaderNext(&reader))) {
          Timestamp next_end = TimeCurrentSystemTime();
          slog_info(0, "got next %zu lines in %f sec",
                    vector != NULL ? VectorLength(vector) : 0,
                    (next_end - next_begin) / 1000.0f);

          StringId *ids = NULL, *ids_out;
          UNUSED(ids_out);

          char **strings = (char **)VectorData(vector);
          size_t num_strings = VectorLength(vector) - 1;

          slog_info(0, "insert..");

          size_t newThreads = DescentCalculate(system_context);
          slog_info(0, "Iteration: %zu", ++iteration)
          slog_info(0, "Resizing Threads to %zu", newThreads);
          StringDictionaryResize(&dic, newThreads, num_buckets, 3720000,
                                 newThreads);

          StringDictionaryResetCounters(&dic);
          Timestamp inserted_begin = TimeCurrentSystemTime();
          // In this case num_threads has to be 0
          StringDictionaryInsert(&dic, &ids, strings, num_strings, 0);
          Timestamp inserted_end = TimeCurrentSystemTime();
          insert_duration = (inserted_end - inserted_begin) / 1000.0f;

          DescentTrain(system_context, insert_duration);

          //                fprintf(stderr, "locate..\n");
          //
          //                string_dic_locate_fast(&ids_out, &dic, strings,
          //                num_strings); for (size_t i = 0; i<num_strings; i++)
          //                {
          //                    StringId id_created = ids[i];
          //                    StringId id_located = ids_out[i];
          //                    //debug("check", "[%s] -> %zu", strings[i],
          //                    id_located);
          //                    PANIC_IF_WARGS(id_created!=id_located,
          //                            "mapping broken for string [%s] id
          //                            '%zu': expected %zu, is %zu",
          //                            strings[i], i, id_created, id_located);
          //                    assert(id_created==id_located);
          //                }
          //
          //                fprintf(stderr, "extract..\n");
          //
          //                char** extracted_strings = string_dic_extract(&dic,
          //                ids, num_strings); for (size_t i = 0; i<num_strings;
          //                i++) {
          //                    char* extracted = extracted_strings[i];
          //                    char* given = strings[i];
          //                    PANIC_IF(strcmp(extracted, given)!=0,
          //                    "extraction broken"); assert(strcmp(extracted,
          //                    given)==0);
          //                    //     debug("extracted id=%zu -> string
          //                    [%s]\n", ids[i], extracted);
          //                }
          //
          //                fprintf(stderr, "remove..\n");

          //  string_dic_remove(&dic, ids, num_strings);

          // struct string_lookup_counters counters;
          // string_dic_counters(&counters, &dic);

          size_t num_distinct;
          StringDictionaryNumDistinct(&num_distinct, &dic);

          total_num += VectorLength(vector);

          slog_info(0, "%zu;%d;%zu;%f;%f;%f;%zu;%zu;%zu", chunk_num++, sample,
                    num_buckets, created_duration, insert_duration,
                    (created_duration + insert_duration), VectorLength(vector),
                    total_num, num_distinct);
          statistics = (stats){statistics.created_duration + created_duration,
                               statistics.insert_duration + insert_duration,
                               statistics.total_duration + created_duration +
                                   insert_duration};

          StringDictionaryFree(&dic, ids);
          // string_dic_free(&dic, extracted_strings);
          // string_dic_free(&dic, ids_out);

          for (size_t i = 0; i < VectorLength(vector); i++) {
            char *string = *VECTOR_GET(vector, i, char *);
            free(string);
          }
          VectorDrop(vector);
          free(vector);
        }

        ChunkReaderDrop(&reader);
        StringDictionaryDrop(&dic);
      }

      statistics = (stats){statistics.created_duration / samples,
                           statistics.insert_duration / samples,
                           statistics.total_duration / samples};
      fprintf(statistics_file, "%zu, %f, %f, %f\n", num_threads,
              statistics.created_duration, statistics.insert_duration,
              statistics.total_duration);
      statistics.created_duration = 0;
      statistics.insert_duration = 0;
      statistics.total_duration = 0;
    }
  }
  fclose(statistics_file);
  exit(0);
}

int main() {
  slog_init("logfile", NULL, 1, 0);
  experiments_hashing();

  return 0;
}
