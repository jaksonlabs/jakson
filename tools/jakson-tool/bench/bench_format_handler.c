
#include "bench_format_handler.h"

bool bench_format_handler_create_carbon(bench_format_handler *handler, bench_carbon_mgr *manager,
                                        bench_error *error, const char* filePath)
{
    // TODO: Implement function
    JAK_ERROR_IF_NULL(handler);
    JAK_ERROR_IF_NULL(manager);

    JAK_UNUSED(error);
    JAK_UNUSED(filePath);

    return false;
}

bool bench_format_handler_create_bson_handler(bench_format_handler *handler, bench_error *error, const char* filePath)
{
    //JAK_ERROR_IF_NULL(handler);

    JAK_UNUSED(error);
    JAK_UNUSED(filePath);
    handler->format_name = "bson";
    error = malloc(sizeof(*error));
    error->msg = NULL;
    error->code = 0;
    handler->error = error;

    bench_bson_error *bsonError = malloc(sizeof(*bsonError));
    bench_bson_mgr *manager = malloc(sizeof(*manager));
    //if(filePath == NULL) {
        bench_bson_error_create(bsonError, error);
        bench_bson_mgr_create_empty(manager, bsonError);
    //} else {
    //    bench_bson_mgr_create_from_file(&manager, filePath);
    //}
    handler->manager = manager;

    return true;
}
/**
bool bench_format_handler_create_ubjson(bench_format_handler *handler, bench_error *error, const char* filePath)
{
    // TODO: Implement function
    JAK_UNUSED(handler);
    JAK_UNUSED(error);
    JAK_UNUSED(filePath);

    return false;
}
**/

bool bench_format_handler_destroy(bench_format_handler *handler)
{
    if(handler == NULL)
        return false;

    if(strcmp(handler->format_name, "carbon") == 0) {
        // TODO: Implement
    } else if(strcmp(handler->format_name, "bson") == 0) {
        if(!bench_bson_mgr_destroy((bench_bson_mgr*) handler->manager))
            return false;
        free(handler->manager);
        free(handler->error);
    } else if(strcmp(handler->format_name, "ubjson") == 0) {
        // TODO: Implement
    } else {
        return false;
    }

    return true;
}

bool bench_format_handler_get_doc(char *str, bench_format_handler *handler) {
    JAK_ERROR_IF_NULL(handler);
    if(strcmp(handler->format_name, "carbon") == 0) {
        // TODO: Implement
        return false;
    } else if(strcmp(handler->format_name, "bson") == 0) {
        return bench_bson_get_doc(str, (bench_bson_mgr*) handler->manager);
    } else if(strcmp(handler->format_name, "ubjson") == 0) {
        // TODO: Implement
        return false;
    } else {
        return false;
    }
}

bool bench_format_handler_insert_int32(bench_format_handler *handler, const char *key, int32_t val)
{
    if(strcmp(handler->format_name, "carbon") == 0) {
        // TODO: Implement
        return false;
    } else if(strcmp(handler->format_name, "bson") == 0) {
        return bench_bson_insert_int32((bench_bson_mgr*) handler->manager, key, val);
    } else if(strcmp(handler->format_name, "ubjson") == 0) {
        // TODO: Implement
        return false;
    } else {
        return false;
    }
}

bool bench_format_handler_find_int32(bench_format_handler *handler, const char *key, int32_t val)
{
    if(strcmp(handler->format_name, "carbon") == 0) {
        // TODO: Implement
        return false;
    } else if(strcmp(handler->format_name, "bson") == 0) {
        return bench_bson_insert_int32((bench_bson_mgr*) handler->manager, key, val);
    } else if(strcmp(handler->format_name, "ubjson") == 0) {
        // TODO: Implement
        return false;
    } else {
        return false;
    }
}

bool bench_format_handler_change_val_int32(bench_format_handler *handler, const char *key, int32_t newVal)
{
    if(strcmp(handler->format_name, "carbon") == 0) {
        // TODO: Implement
        return false;
    } else if(strcmp(handler->format_name, "bson") == 0) {
        return bench_bson_insert_int32((bench_bson_mgr*) handler->manager, key, val);
    } else if(strcmp(handler->format_name, "ubjson") == 0) {
        // TODO: Implement
        return false;
    } else {
        return false;
    }
}

bool bench_format_handler_convert_entry_int32(bench_format_handler *handler, const char *key)
{
    if(strcmp(handler->format_name, "carbon") == 0) {
        // TODO: Implement
        return false;
    } else if(strcmp(handler->format_name, "bson") == 0) {
        return bench_bson_insert_int32((bench_bson_mgr*) handler->manager, key, val);
    } else if(strcmp(handler->format_name, "ubjson") == 0) {
        // TODO: Implement
        return false;
    } else {
        return false;
    }
}

bool bench_format_handler_convert_entry_int64(bench_format_handler *handler, const char *key)
{
    if(strcmp(handler->format_name, "carbon") == 0) {
        // TODO: Implement
        return false;
    } else if(strcmp(handler->format_name, "bson") == 0) {
        return bench_bson_insert_int32((bench_bson_mgr*) handler->manager, key, val);
    } else if(strcmp(handler->format_name, "ubjson") == 0) {
        // TODO: Implement
        return false;
    } else {
        return false;
    }
}

bool bench_format_handler_delete_int32(bench_format_handler *handler, const char *key)
{
    if(strcmp(handler->format_name, "carbon") == 0) {
        // TODO: Implement
        return false;
    } else if(strcmp(handler->format_name, "bson") == 0) {
        return bench_bson_insert_int32((bench_bson_mgr*) handler->manager, key, val);
    } else if(strcmp(handler->format_name, "ubjson") == 0) {
        // TODO: Implement
        return false;
    } else {
        return false;
    }
}
