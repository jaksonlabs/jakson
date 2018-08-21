#include <ng5/result_handle.h>

int result_handle_create(struct result_handle *handle, struct storage_engine *context, slot_vector_slot_t id,
        const struct vector of_type(T) *result, enum result_handle_type type)
{
    check_non_null(handle);
    check_non_null(context);
    handle->result = result;
    handle->type = type;
    handle->id = id;
    handle->context = context;
    return STATUS_OK;
}

const void *result_handle_read(size_t *num_elements, const struct result_handle *handle)
{
    //check_non_null(handle);
    //check_non_null(num_elements);
    unused(num_elements);
    return vector_data(handle->result);
}

int result_handle_drop(struct result_handle *handle)
{
    check_non_null(handle);
    unused(handle);
    return STATUS_OK;
}