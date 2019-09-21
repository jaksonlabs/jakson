
#include "bench_bson.h"
#include "bench_format_handler.h"

bool bench_bson_error_create(bench_bson_error *bsonError, bench_error *benchError) {
    bsonError->err = benchError;
    return 0;
}



bool bench_bson_error_write(bench_bson_error *error, char *msg, size_t errOffset) {
    if(errOffset) {
        error->err->msg = strcat(msg, (const char *) errOffset);
    } else {
        error->err->msg = msg;
    }
    return true;
}

bool bench_bson_mgr_create_from_file(bench_bson_mgr *manager, const char* filePath)
{
    //JAK_ERROR_IF_NULL(manager);
    //JAK_ERROR_IF_NULL(error);
    JAK_UNUSED(filePath);
    bson_t b = BSON_INITIALIZER;
    bson_json_reader_t *jReader;
    bench_bson_error *error = malloc(sizeof(bench_bson_error));
    bson_error_t bError;

    if(!(jReader = bson_json_reader_new_from_file (filePath, &bError))) {
        char* msg = strcat("BSON reader failed to open: ", filePath);
        msg = strcat(msg, strcat("\n", bError.message));
        manager->error->err->msg = msg;
        return false;
    }

    int readResult;
    while((readResult = bson_json_reader_read (jReader, &b, &bError))) {
        if(readResult < 0) {
            char* msg = strcat("Error in JSON parsing:\n", bError.message);
            manager->error->err->msg = msg;
            return false;
        }
    }
    size_t errOffset;
    if(!bson_validate(&b, BSON_VALIDATE_NONE, &errOffset))
        return false;
    bson_json_reader_destroy(jReader);
    manager->b = &b;
    //error->bError = bError;
    manager->error = error;
    bson_iter_t *it = malloc(sizeof(bson_iter_t));
    if (!bson_iter_init(it, manager->b))
        return false;
    manager->it = it;

    return true;
}

bool bench_bson_mgr_create_empty(bench_bson_mgr *manager, bench_bson_error *error)
{
    JAK_ERROR_IF_NULL(manager);
    JAK_ERROR_IF_NULL(error);

    bson_t *b = bson_new();
    manager->b = b;
    manager->error = error;
    return true;
}

bool bench_bson_mgr_destroy(bench_bson_mgr *manager)
{
    bson_destroy(manager->b);
    return true;
}

bool bench_bson_get_doc(char* str, bench_bson_mgr *manager) {
    size_t errOffset;
    bson_iter_t it;
    if (!bson_validate(manager->b, BSON_VALIDATE_NONE, &errOffset)) {
        bench_bson_error_write(manager->error, "The document failed to validate at offset: %u\n", errOffset);
        return false;
    }
    if (bson_iter_init(&it, manager->b)) {
        while(bson_iter_next(&it)) {
            str = strcat(str, bson_iter_key(&it));
        }
    } else {
        bench_bson_error_write(manager->error, "Failed to initialize Iterator. Aborting...", 0);
        return false;
    }
    return str;
}

bool bench_bson_insert_int32(bench_bson_mgr *manager, const char *key, int32_t val)
{
    bson_append_int32(manager->b, key, strlen(key), val);
    return true;
}
