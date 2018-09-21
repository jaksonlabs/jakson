#include <stdio.h>
#include <iostream>
#include <vector.h>
#include <time.h>
#include <map>
#include <vector>

using namespace std;


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
    ng5_vector_t *vector = static_cast<ng5_vector_t*>(malloc(sizeof(ng5_vector_t)));
    ng5_vector_create(vector, NULL, sizeof(char*), 15372804);
    char *begin, *end;
    begin = (char *) contents;
    for (end = (char *) contents; *end != '\0'; end++) {
        size_t len = (end - begin);
        if (unlikely(*end == '\n' && len > 0)) {
            char *string = static_cast<char*>(malloc(len + 1));
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


using namespace std;

template<typename K = string, typename V = string_id_t, typename S = size_t> class StrDic
{
public:
    using KeyType = K;
    using ValueType = V;
    using SizeType = S;

private:
    map<KeyType, ValueType>     mapping;
    vector<KeyType>             inverted_index;

public:
    StrDic(size_t reserve): mapping(), inverted_index()
    {
        inverted_index.reserve(reserve);
    }

    ~StrDic()
    {
        mapping.clear();
        inverted_index.clear();
    }

    vector<ValueType> insert(const vector<KeyType> &keys, SizeType num_keys)
    {
        auto result = vector<ValueType>();
        result.reserve(num_keys);

        auto it = keys.begin();
        while (num_keys--) {
            auto key = *it++;
            auto it  = mapping.find(key);
            if (it != mapping.end()) {
                result.push_back(it->second);
            } else {
                auto id = inverted_index.size();
                result.push_back(id);
                inverted_index.push_back(key);
                mapping.insert(pair<KeyType, ValueType>(key, id));
            }
        }

        return move(result);
    }

    vector<ValueType> locate_fast(const vector<KeyType> &strings, SizeType num_strings) {
        auto result = vector<ValueType>();
        result.reserve(num_strings);
        auto it = strings.begin();
        while(num_strings--) {
            result.push_back(mapping.find(*it++)->second);
        }

        return move(result);
    }

    vector<KeyType> extract_fast(const vector<ValueType>& ids) {
        auto num_ids = ids.size();
        auto result = vector<KeyType>();
        result.reserve(num_ids);

        for (auto i = 0; i < num_ids; i++) {
            result.push_back(inverted_index[i]);
        }
        return move(result);
    }
};

size_t file_size(const char *path) {
    FILE *f = fopen(path, "r");
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

#define NUM_SAMPLES 50

int main(void) {

    printf("file;file_size_b;file_size_gbit,sample;created_duration_ms;insert_duration_ms;locate_full_duration_ms;extract_full_duration_ms;num_strings;"
            "created_duration_Gbps;insert_duration_Gbps;locate_full_duration_Gbps;extract_full_duration_Gbps\n");

    const char *file = "/Volumes/PINNECKE EXT/science/datasets/mag-open-academic-graph/stringlists/catted.txt";
    //const char *file = "/Volumes/PINNECKE EXT/science/datasets/yago/datasets/rdf3x/yago1.n3/samples-stringlist/yago1-11pc-stringlist.txt";

    size_t size = file_size(file);
    float size_gbit = (size * 8) / 1000000000.0;
    char* contents = read_contents(file);
    ng5_vector_t* lines = to_string_list(contents);

    for (int sample = 0; sample<NUM_SAMPLES; sample++) {
        float created_duration = 0;
        float insert_duration = 0;
        float locate_full_duration = 0;
        float extract_full_duration = 0;

        char** strings = (char**) ng5_vector_data(lines);
        size_t num_strings = ng5_vector_len(lines);
        auto stl_strings = vector<string>();
        stl_strings.reserve(num_strings);
        for (size_t i = 0; i < num_strings; i++) {
            stl_strings.push_back(string(strings[i]));
        }

        timestamp_t create_begin = time_current_time_ms();
        StrDic<> cxx_dic(num_strings);
        timestamp_t create_end = time_current_time_ms();
        created_duration = (create_end-create_begin);

        timestamp_t inserted_begin = time_current_time_ms();
        auto ids = cxx_dic.insert(stl_strings, num_strings);
        timestamp_t inserted_end = time_current_time_ms();
        insert_duration = (inserted_end-inserted_begin);

        timestamp_t locate_begin = time_current_time_ms();
        auto ids_out = cxx_dic.locate_fast(stl_strings, num_strings);
        timestamp_t locate_end = time_current_time_ms();
        locate_full_duration = (locate_end-locate_begin);

#ifndef NDEBUG
        for (size_t i = 0; i < num_strings; i++) {
            StringId id_created = ids[i];
            StringId id_located = ids_out[i];
            //debug("check", "[%s] -> %zu", strings[i], id_located);
            panic_if_wargs(id_created != id_located, "mapping broken for string [%s] id '%zu': expected %zu, is %zu", strings[i], i, id_created, id_located);
            assert(id_created == id_located);
        }
#endif

        timestamp_t extract_begin = time_current_time_ms();
        auto extracted_strings = cxx_dic.extract_fast(ids_out);
        timestamp_t extract_end = time_current_time_ms();
        extract_full_duration = (extract_end-extract_begin);

        printf("%s;%zu;%f;%d;%f;%f;%f;%f;%zu;%f;%f;%f;%f;\n",
                file,
                size,
                size_gbit,
                sample,
                created_duration,
                insert_duration,
                locate_full_duration,
                extract_full_duration,
                ng5_vector_len(lines),
                size_gbit/(created_duration/1000.0f),
                size_gbit/(insert_duration/1000.0f),
                size_gbit/(locate_full_duration/1000.0f),
                size_gbit/(extract_full_duration/1000.0f)
        );
    }

    free(contents);
    ng5_vector_drop(lines);


    fflush(stderr);
    fflush(stdout);

}