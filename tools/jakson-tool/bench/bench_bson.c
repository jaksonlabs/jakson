
#include "bench_bson.h"

bool bench_bson_mgr_create_from_file(bench_bson_mgr *manager, const char* filePath)
{
    //JAK_ERROR_IF_NULL(manager);
    //JAK_ERROR_IF_NULL(error);
    JAK_UNUSED(filePath);

    bson_t b = BSON_INITIALIZER;
    bson_json_reader_t *bReader;
    bson_error_t bError;

    if(!(bReader = bson_json_reader_new_from_file (filePath, &bError))) {
        char* msg = strcat("BSON reader failed to open: ", filePath);
        msg = strcat(msg, strcat("\n", bError.message));
        manager->error->err.msg = msg;
        return false;
    }

    int readResult;
    while((readResult = bson_json_reader_read (bReader, &b, &bError))) {
        if(readResult < 0) {
            char* msg = strcat("Error in JSON parsing:\n", bError.message);
            manager->error->err.msg = msg;
            return false;
        }
    }
    bson_json_reader_destroy(bReader);
    manager->b = &b;
    manager->error->bError = bError;
    return true;
}

bool bench_bson_mgr_create_empty(bench_bson_mgr *manager, bench_bson_error *error)
{
    //JAK_ERROR_IF_NULL(manager);
    JAK_ERROR_IF_NULL(error);

    bson_t b;
    bson_init(&b);
    manager->b = &b;
    manager->error = error;
    return true;
}

bool bench_bson_mgr_destroy(bench_bson_mgr *manager)
{
    bson_destroy(manager->b);
    return true;
}

bool bench_bson_mgr_insert_int32(bench_bson_mgr *manager, const char *key, uint32_t val)
{
    bson_append_int32(manager->b, key, strlen(key), val);
    return true;
}