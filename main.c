#include <stdio.h>
#include <stdx/ng5_string_map.h>
#include <stdx/ng5_time.h>
#include <stdlib.h>
#include <stdx/ng5_string_dic_sync.h>
#include <stdx/ng5_string_dic_async.h>
#include <apr_general.h>
//#include <ng5/roadfire/roadfire.h>

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

ng5_vector_t *to_string_list(const char *contents)
{
    fprintf(stderr, "converting to line list...");
    ng5_vector_t *vector = malloc(sizeof(ng5_vector_t));
    ng5_vector_create(vector, NULL, sizeof(char*), 15372804);
    char *begin, *end;
    begin = (char *) contents;
    for (end = (char *) contents; *end != '\0'; end++) {
        size_t len = (end - begin);
        if (unlikely(*end == '\n' && len > 0)) {
            char *string = malloc(len + 1);
            memcpy(string, begin, len);
            string[len] = '\0';

            ng5_vector_push(vector, &string, 1);
            begin = end + 1;

        }
       // fprintf(stderr, "%f done so far...\n", vector_len(ng5_vector) / 4561977.0f * 100);
    }
    fprintf(stderr, "DONE, %zu lines\n", ng5_vector_len(vector));
    return vector;
}

#define NUM_SAMPLES 2
#define NTHREADS   256

/*void roadfire_test() {
    struct storage_engine engine;
    storage_engine_roadfire_create(&engine, NULL, NULL);
}*/

void experiments_hashing()
{

    printf("yago_percent;sample;num_buckets;time_created_sec;time_inserted_sec;time_bulk_sum_created_inserted;num_strings\n");

    const char* paths[11];
    paths[0] = "/Users/marcus/Downloads/50.txt";

//      paths[0] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/100.txt";
    /*    paths[0] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-15pc-stringlist.txt";
         paths[1] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-19pc-stringlist.txt";
          paths[2] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-27pc-stringlist.txt";
          paths[3] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-35pc-stringlist.txt";
          paths[4] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-43pc-stringlist.txt";
          paths[5] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-51pc-stringlist.txt";
          paths[6] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-59pc-stringlist.txt";
          paths[7] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-67pc-stringlist.txt";
          paths[8] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-75pc-stringlist.txt";
          paths[9] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-83pc-stringlist.txt";
          paths[10] = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-91pc-stringlist.txt";
    */

    paths[0] = "/home/pinnecke/datasets/yago1/stringlists/yago1-15pc-stringlist.txt";
    paths[1] = "/home/pinnecke/datasets/yago1/stringlists/yago1-19pc-stringlist.txt";
    paths[2] = "/home/pinnecke/datasets/yago1/stringlists/yago1-27pc-stringlist.txt";
    paths[3] = "/home/pinnecke/datasets/yago1/stringlists/yago1-35pc-stringlist.txt";
    paths[4] = "/home/pinnecke/datasets/yago1/stringlists/yago1-43pc-stringlist.txt";
    paths[5] = "/home/pinnecke/datasets/yago1/stringlists/yago1-51pc-stringlist.txt";
    paths[6] = "/home/pinnecke/datasets/yago1/stringlists/yago1-59pc-stringlist.txt";
    paths[7] = "/home/pinnecke/datasets/yago1/stringlists/yago1-67pc-stringlist.txt";
    paths[8] = "/home/pinnecke/datasets/yago1/stringlists/yago1-75pc-stringlist.txt";
    paths[9] = "/home/pinnecke/datasets/yago1/stringlists/yago1-83pc-stringlist.txt";
    paths[10] = "/home/pinnecke/datasets/yago1/stringlists/yago1-91pc-stringlist.txt";

int yago_percent[11] = {
  15,
  19,
  27,
  35,
  43,
  51,
  59,
  67,
  75,
  83,
  91
};


for (int pi = 0; pi<11; pi++) {

timestamp_t read_begin = time_current_time_ms();
char* contents = read_contents(paths[pi]);
fprintf(stderr, "processing '%s'\n", paths[pi]);
timestamp_t read_end = time_current_time_ms();
fprintf(stderr, "... %fsec\n", (read_end-read_begin)/1000.0f);

timestamp_t convert_begin = time_current_time_ms();
ng5_vector_t* lines = to_string_list(contents);
size_t num_lines = ng5_vector_len(lines);
timestamp_t convert_end = time_current_time_ms();
fprintf(stderr, "... %fsec\n", (convert_end-convert_begin)/1000.0f);
fflush(stderr);

struct string_dic dic;

for (size_t num_buckets = 50000; num_buckets<=50000; num_buckets += 50000) {
  for (int sample = 0; sample<NUM_SAMPLES; sample++) {

      float created_duration = 0;
      float insert_duration = 0;

      fprintf(stderr, "*** %d of %d in progress ***\n", sample+1, NUM_SAMPLES);

      fprintf(stderr, "create..\n");

      timestamp_t create_begin = time_current_time_ms();
      string_dic_create_async(&dic, ng5_vector_len(lines), num_buckets, num_lines, NTHREADS, NULL);                         // <--------------------------------------------
      timestamp_t create_end = time_current_time_ms();
      created_duration = (create_end-create_begin)/1000.0f;

      string_id_t* ids = NULL, * ids_out;
      unused(ids_out);

      char** strings = (char**) ng5_vector_data(lines);
      size_t num_strings = ng5_vector_len(lines)-1;


      fprintf(stderr, "insert..\n");

      string_dic_reset_counters(&dic);
      timestamp_t inserted_begin = time_current_time_ms();
      string_dic_insert(&dic, &ids, strings, num_strings, 0);
      string_dic_insert(&dic, &ids, strings, num_strings, 0); // HACK: Test for bulk existence
      timestamp_t inserted_end = time_current_time_ms();
      insert_duration = (inserted_end-inserted_begin)/1000.0f;

      fprintf(stderr, "locate..\n");

      string_dic_locate_fast(&ids_out, &dic, strings, num_strings);
      for (size_t i = 0; i < num_strings; i++) {
          string_id_t id_created = ids[i];
          string_id_t id_located = ids_out[i];
          //debug("check", "[%s] -> %zu", strings[i], id_located);
          panic_if_wargs(id_created != id_located, "mapping broken for string id '%zu': expected %zu, is %zu", i, id_created, id_located);
          assert(id_created == id_located);
      }

      fprintf(stderr, "extract..\n");

      char **extracted_strings = string_dic_extract(&dic, ids, num_strings);
      for (size_t i = 0; i < num_strings; i++) {
          char *extracted = extracted_strings[i];
          char *given     = strings[i];
          panic_if(strcmp(extracted, given) != 0, "extraction broken");
          assert(strcmp(extracted, given) == 0);
     //     debug("extracted id=%zu -> string [%s]\n", ids[i], extracted);
      }

      fprintf(stderr, "remove..\n");

     //  string_dic_remove(&dic, ids, num_strings);

      //struct string_lookup_counters counters;
      //string_dic_counters(&counters, &dic);

      string_dic_free(&dic, ids);
      string_dic_drop(&dic);

      printf("%d;%d;%zu;%f;%f;%f;%zu\n", yago_percent[pi], sample, num_buckets, created_duration,
              insert_duration,
              (created_duration+insert_duration), ng5_vector_len(lines));

      fflush(stderr);
      fflush(stdout);

  }
}

free(contents);

for (size_t i = 0; i <ng5_vector_len(lines); i++) {
  char *string = *ng5_vector_get(lines, i, char *);
  free(string);
}

ng5_vector_drop(lines);
free(lines);

}
}

int main()
{
//roadfire_test();
experiments_hashing();

return 0;
}