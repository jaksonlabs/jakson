
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

bool bench_format_handler_create_bson(bench_format_handler *handler, bench_error *error, const char* filePath)
{
    //JAK_ERROR_IF_NULL(handler);

    JAK_UNUSED(error);
    JAK_UNUSED(filePath);

    handler->format_name = "bson";
    handler->error = error;

    bench_bson_error *bsonError = malloc(sizeof(bench_bson_error));
    bench_bson_mgr *manager = malloc(sizeof(bench_bson_mgr));
    if(manager == NULL)
        return false;
    bench_bson_mgr_create_from_file(manager, filePath);

    manager->error = bsonError;
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
        free(handler->format_name);
        free(handler->error);
        if(!bench_bson_mgr_destroy((bench_bson_mgr*) handler->manager))
            return false;
        free(handler->manager);
    } else if(strcmp(handler->format_name, "ubjson") == 0) {
        // TODO: Implement
    } else {
        return false;
    }

    return true;
}

bool bench_format_handler_insert_int32(bench_format_handler *handler, const char *key, uint32_t val)
{
    if(strcmp(handler->format_name, "carbon") == 0) {
        // TODO: Implement
    } else if(strcmp(handler->format_name, "bson") == 0) {
        return bench_bson_mgr_insert_int32((bench_bson_mgr*) handler->manager, key, val);
    } else if(strcmp(handler->format_name, "ubjson") == 0) {
        // TODO: Implement
    } else {
        return false;
    }
}

