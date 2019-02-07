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

bool test_resize() {
  StringDictionary dic;

  size_t num_buckets = 50000;
  size_t num_threads = 4;

  StringDictionaryCreateAsync(&dic, 3720000, num_buckets, 3720000,
                              num_threads, NULL);

  num_threads = 8;
  slog_info(0, "Resizing to %zu", num_threads);
  StringDictionaryResize(&dic, 3720000, num_buckets, 3720000, num_threads);

  num_threads = 2;
  slog_info(0, "Resizing to %zu", num_threads);
  StringDictionaryResize(&dic, 3720000, num_buckets, 3720000, num_threads);
  StringDictionaryDrop(&dic);

  return true;
}

//void unit_test() {
//   fprintf(stderr, "locate..\n");
//
//   string_dic_locate_fast(&ids_out, &dic, strings,
//   num_strings); for (size_t i = 0; i<num_strings; i++)
//   {
//       StringId id_created = ids[i];
//       StringId id_located = ids_out[i];
//       debug("check", "[%s] -> %zu", strings[i],
//       id_located);
//       PANIC_IF_WARGS(id_created!=id_located,
//               "mapping broken for string [%s] id
//               '%zu': expected %zu, is %zu",
//               strings[i], i, id_created, id_located);
//       assert(id_created==id_located);
//   }
//
//   fprintf(stderr, "extract..\n");
//
//   char** extracted_strings = string_dic_extract(&dic,
//   ids, num_strings); for (size_t i = 0; i<num_strings;
//   i++) {
//       char* extracted = extracted_strings[i];
//       char* given = strings[i];
//       PANIC_IF(strcmp(extracted, given)!=0,
//       "extraction broken"); assert(strcmp(extracted,
//       given)==0);
//       debug("extracted id=%zu -> string[%s]\n", ids[i], extracted);
//   }
//
//   fprintf(stderr, "remove..\n");
//
//   string_dic_remove(&dic, ids, num_strings);
//
//   struct string_lookup_counters counters;
//   string_dic_counters(&counters, &dic);
//}

void benchmark(const size_t limit, const char* path, const size_t samples, const char* method, size_t (*eval) (context sys_cont, float performance)) {

  (void) limit;
  char* postfix = "_statistics.csv";
  char* file_name = malloc(strlen(method)+strlen(postfix));
  strcpy(file_name, method);
  strcat(file_name, postfix);
  FILE *statistics_file = fopen(file_name, "w");
  free(file_name);
  //FILE *thread_file = fopen("threads.csv", "w");
  stats statistics = {0.0, 0.0, 0.0};
  context system_context = {0, 0, 0, 0};
  float performance;

  fprintf(statistics_file, "%s, %s, %s, %s, %s\n", "thread_num", "created_duration",
          "insert_duration", "total_time", "chunk_size");

  system_info(path, &system_context, BATCH_SIZE);

  StringDictionary dic;
    for (size_t num_buckets = 50000; num_buckets <= 50000;
         num_buckets += 50000) {
      for (size_t sample = 1; sample <= samples; sample++) {

        slog_info(0, "PERFORMING SAMPLE %zu", sample);

        ChunkReader reader;

        ChunkReaderCreate(&reader, NULL, path, BATCH_SIZE);

        float created_duration = 0;
        float insert_duration = 0;

        slog_info(0, "*** %d of %d in progress ***", sample, samples);
        slog_info(0, "create..");

        Timestamp create_begin = TimeCurrentSystemTime();
        StringDictionaryCreateAsync(&dic, 3720000, num_buckets, 3720000,
                                    system_context.processor_number, NULL);
        Timestamp create_end = TimeCurrentSystemTime();
        created_duration = (create_end - create_begin) / 1000.0f;

        Timestamp next_begin = TimeCurrentSystemTime();
        Vector ofType(char *) * vector;

        //size_t chunk_num = 0;
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

          size_t newThreads = eval(system_context, performance);
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

          performance = insert_duration;

          size_t num_distinct;
          StringDictionaryNumDistinct(&num_distinct, &dic);

          total_num += VectorLength(vector);

          statistics = (stats){statistics.created_duration + created_duration,
                               statistics.insert_duration + insert_duration,
                               statistics.total_duration + created_duration +
                                   insert_duration};

          fprintf(statistics_file, "%zu, %f, %f, %f, %zu\n", newThreads, created_duration, insert_duration, created_duration + insert_duration, BATCH_SIZE);

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
      statistics.created_duration = 0;
      statistics.insert_duration = 0;
      statistics.total_duration = 0;
    }
  fclose(statistics_file);
}

char init_check = 0;

size_t descent_eval(context sys_cont, float performance) {
  if (init_check == 0) {
    DescentInit();
    init_check = 1;
  }

  DescentTrain(sys_cont, performance);
  return DescentCalculate(sys_cont);
}

size_t static_eval(context sys_cont, float performance) {
  (void) performance;
  return sys_cont.processor_number;
}

int main() {
  slog_init("logfile", NULL, 1, 0);

  slog_info(0, "Testing Basic Functionalities");

  if (!test_resize()){
    slog_panic(0, "Testing failed");
    return 1;
  }

  slog_info(0, "Reading in environment");
  const char *limit_string = getenv("NG5_LIMIT");
  if (limit_string == NULL) {
    slog_panic(0, "No sample number defined; Define NG5_LIMIT");
  }
  const size_t limit = (size_t)atoi(limit_string);

  const char *path = getenv("NG5_SET");
  if (path == NULL) {
    slog_panic(0, "No dataset defined; Define NG5_SET");
    exit(1);
  }

  const char *samples_string = getenv("NG5_SAMPLES");
  if (samples_string == NULL) {
    slog_panic(0, "No sample number defined; Define NG5_SAMPLES");
  }
  const size_t samples = (size_t)atoi(samples_string);


  benchmark(limit, path, samples, "descent", descent_eval);

  benchmark(limit, path, samples, "static", static_eval);

  return 0;
}
