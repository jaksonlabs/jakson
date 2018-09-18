#include <utils/ng5_chunked_reader.h>
#include <errno.h>

int ng5_chunked_reader_create(ng5_chunked_reader_t *reader, ng5_allocator_t *alloc,
        const char *file_path, size_t chunk_size_threshold)
{
    check_non_null(reader)
    check_non_null(file_path)
    check_non_null(alloc)
    check_non_null(chunk_size_threshold)

    allocator_this_or_default(&reader->alloc, alloc);
    reader->file_path = strdup(file_path);
    reader->chunk_size_threshold = chunk_size_threshold;
    reader->file = fopen(file_path, "r");
    reader->offset = 0;

    if (reader->file == NULL) {
        perror(strerror(errno));
        ng5_chunked_reader_drop(reader);
        return STATUS_FAILED;
    } else {
        fseek(reader->file, 0L, SEEK_END);
        reader->file_size = ftell(reader->file);
        fseek(reader->file, 0L, SEEK_SET);
    }

    return STATUS_OK;
}

int ng5_chunked_reader_drop(ng5_chunked_reader_t *reader)
{
    check_non_null(reader);
    free(reader->file_path);
    if (reader->file != NULL) {
        fclose(reader->file);
        reader->file = NULL;
    }
    return STATUS_OK;
}

static ng5_vector_t *to_string_list(const char *contents, size_t num_bytes)
{
    fprintf(stderr, "converting to line list...");
    ng5_vector_t *vector = malloc(sizeof(ng5_vector_t));
    ng5_vector_create(vector, NULL, sizeof(char*), 15372804);
    char *begin, *end;
    begin = (char *) contents;
    end = (char *) contents + num_bytes;
    while(*begin != '\0') {

        char *it;
        for (it = begin; it < end; it++) {
            if (*it == '\n') {
                break;
            }
        }

        size_t len = it - begin;
        if (len > 0) {
            char* string = malloc(len+1);
            memcpy(string, begin, len);
            string[len] = '\0';
            ng5_vector_push(vector, &string, 1);
            begin += len;
        } else {
            begin++;
        }
    }
    fprintf(stderr, "DONE, %zu lines\n", ng5_vector_len(vector));
    return vector;
}

ng5_vector_t of_type(char *) *ng5_chunked_reader_next(ng5_chunked_reader_t *reader)
{
    check_non_null(reader);
    check_non_null(reader->file);

    if (reader->offset >= reader->file_size) {
        return NULL;
    } else {
        fseek(reader->file, reader->offset, SEEK_SET);

        char *buffer = (char*)malloc(reader->chunk_size_threshold * sizeof(char) + 1);

        if(buffer == NULL)
            return NULL;

        size_t bytes_to_read = min(reader->chunk_size_threshold, (reader->file_size - reader->offset));
        fread(buffer, sizeof(char), bytes_to_read, reader->file);

        reader->offset += bytes_to_read;

        fprintf(stderr, "input read, progress %f%%\n", reader->offset * 100 / (float) reader->file_size);

        buffer[bytes_to_read] = '\0';

        fprintf(stderr, "DONE\n");

        ng5_vector_t of_type(char *) *result = to_string_list(buffer, bytes_to_read);
        free (buffer);

        return result;
    }
}