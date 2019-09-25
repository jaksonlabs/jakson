
#include "bench_bson.h"
#include "bench_format_handler.h"

bool bench_bson_error_create(bench_bson_error *bsonError, bench_error *benchError) {
    bsonError->err = benchError;
    return 0;
}

// TODO : Use error write function implicitly by handling error messages in-function as additional parameter
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

bool bench_bson_insert_int32(bench_bson_mgr *manager, bson_iter_t *it, const char *key, int32_t val)
{
    JAK_ERROR_IF_NULL(manager);
    if(!it) {
        return bson_append_int32(manager->b, key, strlen(key), val);
    } else {
        // TODO? : Insert at current iterator position
        return false;
    }
}

bool bench_bson_find_int32(bench_bson_mgr *manager, bson_iter_t *it, const char *key, int32_t val) {
    JAK_ERROR_IF_NULL(manager);
    JAK_UNUSED(val);

    return bson_iter_find(it, key);
}

bool bench_bson_change_val_int32(bench_bson_mgr *manager, bson_iter_t *it, const char *key, int32_t newVal) {
    JAK_ERROR_IF_NULL(manager);
    JAK_UNUSED(key);

    bson_iter_overwrite_int32(it, newVal);

    return true;
}

bool bench_bson_convert_entry_int32(bench_bson_mgr *manager, bson_iter_t *it, const char *key) {
    JAK_ERROR_IF_NULL(manager);
    JAK_ERROR_IF_NULL(key);
    JAK_UNUSED(it);

    bson_iter_t itStart;
    bson_t *bNew = bson_new();
    /*
    if(bson_iter_find(it, key)) {
        uint8_t *newDoc = malloc(sizeof(manager->b->padding) - sizeof(bson_iter_type(it)));
        memcpy(newDoc, manager->b->padding, bson_iter_offset(it));
    } else {
        return false;
    }
    */
    if (bson_iter_init(&itStart, manager->b)) {
        while (bson_iter_next (&itStart)) {
            if (!strcmp(key, bson_iter_key(&itStart))) {
                if (!bson_append_iter (bNew, NULL, 0, &itStart))
                    return false;
            } else {
                if (!bson_append_int32(bNew, key, strlen(key), (int32_t) bson_iter_value(&itStart)))
                    return false;
                bson_iter_init_from_data_at_offset(it, bson_get_data(bNew), bNew->len,
                                                   bson_iter_offset(&itStart), bson_iter_key_len(&itStart));
            }
        }
    }
    bson_free(manager->b);
    manager->b = bNew;
    return true;
}

bool bench_bson_convert_entry_int64(bench_bson_mgr *manager, bson_iter_t *it, const char *key) {
    JAK_ERROR_IF_NULL(manager);
    JAK_ERROR_IF_NULL(key);
    JAK_UNUSED(it);

    bson_iter_t itStart;
    bson_t *bNew = bson_new();
    if (bson_iter_init(&itStart, manager->b)) {
        while (bson_iter_next (&itStart)) {
            if (strcmp(key, bson_iter_key(&itStart))) {
                if (!bson_append_iter (bNew, NULL, 0, &itStart))
                    return false;
            } else {
                if (!bson_append_int64(bNew, key, strlen(key), (int64_t) bson_iter_value(&itStart)))
                    return false;
                bson_iter_init_from_data_at_offset(it, bson_get_data(bNew), bNew->len,
                                                    bson_iter_offset(&itStart), bson_iter_key_len(&itStart));
            }
        }
    }
    bson_free(manager->b);
    manager->b = bNew;
    return true;
}

bool bench_bson_delete_int32(bench_bson_mgr *manager, bson_iter_t *it, const char *key) {
    JAK_ERROR_IF_NULL(manager);
    JAK_UNUSED(it);

    bson_t *bNew= bson_new();
    bson_copy_to_excluding_noinit(manager->b, bNew, key, NULL);

    return true;
}

bool bench_bson_execute_benchmark(bench_bson_mgr *manager, const char *benchType) {
    JAK_ERROR_IF_NULL(manager);
    JAK_UNUSED(benchType);

    assert(bench_bson_insert_int32(manager, 0, "Test1", 41));
    assert(bench_bson_insert_int32(manager, 0, "Test2", 42));
    assert(bench_bson_insert_int32(manager, 0, "Test3", 43));
    assert(bench_bson_insert_int32(manager, 0, "Test4", 44));
    assert(bench_bson_insert_int32(manager, 0, "Test5", 45));
    assert(bench_bson_insert_int32(manager, 0, "Test6", 46));

    bson_iter_t it;
    bson_iter_init(&it, manager->b);

    if(!bench_bson_find_int32(manager, &it, "Test3", 0))
        return bench_bson_error_write(manager->error, "Failed to find int32 value.", 0);

    if(!bench_bson_change_val_int32(manager, &it, 0, 21))
        return bench_bson_error_write(manager->error, "Failed to change int32 value.", 0);

    if(!bench_bson_convert_entry_int64(manager, &it, "Test4"))
        return bench_bson_error_write(manager->error, "Failed to convert to int64 entry.", 0);

    if(!bench_bson_convert_entry_int32(manager, &it, "Test4"))
        return bench_bson_error_write(manager->error, "Failed to convert to int32 entry.", 0);

    if(!bench_bson_delete_int32(manager, &it, "Test4"))
        return bench_bson_error_write(manager->error, "Failed to delete int32 entry.", 0);

    return true;
}
