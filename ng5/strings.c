#include <ng5/strings.h>
#include <stdx/asnyc.h>
#include <stdx/string_id_map.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

struct naive_extra_t {
    struct spinlock      spinlock;
    struct vector        content;
    struct string_id_map inverted_index;
    struct vector        content_freelist;
    string_id_t          next_string_id;
};

static int naive_drop(struct string_pool *self);

static int naive_insert(struct string_pool *self, struct string *out, size_t *num_out,
                                const char **strings, size_t num_strings);

static int naive_remove(struct string_pool *self, struct string *strings, size_t num_strings);

static int naive_find_by_string(struct string_pool *self, struct string *out, size_t *num_out,
                                        const char **strings, size_t num_strings);

static int naive_find_by_id(struct string_pool *self, const char **strings, size_t *num_out,
                            const string_id_t *ids, size_t num_ids);

static int naive_resolve(struct string_pool *self, char **out, size_t *num_out, struct string *strings,
                                 size_t num_strings);

static inline struct naive_extra_t *naive_extra(struct string_pool *self);

static void naive_setup(struct string_pool* self);

static void naive_lock(struct string_pool *self);

static void naive_unlock(struct string_pool *self);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

int string_pool_create_naive(struct string_pool *pool, size_t capacity, const struct allocator *alloc)
{
    // assign allocator which will be used for memory management
    check_success(allocator_this_or_default(&pool->alloc, alloc));

    // assign implementation-specific functions
    pool->drop =            naive_drop;
    pool->insert =          naive_insert;
    pool->remove =          naive_remove;
    pool->find_by_string =  naive_find_by_string;
    pool->find_by_id =      naive_find_by_id;
    pool->resolve =         naive_resolve;

    // define this implementation as 'naive' implementation (e.g., checked by 'naive_extra()' function)
    pool->tag = SP_NAIVE;

    // setup naive-implementation-specific structures
    pool->extra = allocator_malloc(&pool->alloc, sizeof(struct naive_extra_t));
    struct naive_extra_t *extra = naive_extra(pool);
    check_success(spinlock_create(&extra->spinlock));
    check_success(vector_create(&extra->content, alloc, sizeof(char *), capacity));
    check_success(string_id_map_create_simple(&extra->inverted_index, alloc, capacity / 2, 2, 1.7f));
    check_success(vector_create(&extra->content_freelist, alloc, sizeof(string_id_t), capacity));
    extra->next_string_id = 0;
    naive_setup(pool);

    return STATUS_OK;
}

int string_pool_drop(struct string_pool *pool)
{
    check_non_null(pool);
    return pool->drop(pool);
}

int string_pool_insert(struct string *out, size_t *num_out, struct string_pool *pool,
        const char **strings, size_t num_strings)
{
    check_non_null(pool);
    check_non_null(strings);
    assert(pool->insert);
    return pool->insert(pool, out, num_out, strings, num_strings);
}

int string_pool_remove(struct string_pool *pool, struct string *strings, size_t num_strings)
{
    check_non_null(pool);
    check_non_null(strings);
    assert(pool->remove);
    return pool->remove(pool, strings, num_strings);
}

int string_pool_find_by_string(struct string *out, size_t *num_out, struct string_pool *pool,
        const char **strings, size_t num_strings)
{
    check_non_null(out);
    check_non_null(num_out);
    check_non_null(pool);
    check_non_null(strings);
    assert(pool->find_by_string);
    return pool->find_by_string(pool, out, num_out, strings, num_strings);
}

int string_pool_find_by_id(const char **strings, size_t *num_out, struct string_pool *pool,
                           const string_id_t *ids, size_t num_ids)
{
    check_non_null(strings);
    check_non_null(num_out);
    check_non_null(pool);
    check_non_null(ids);

    assert(pool->find_by_id);
    return pool->find_by_id(pool, strings, num_out, ids, num_ids);
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static int naive_drop(struct string_pool *self)
{
    struct naive_extra_t *extra = naive_extra(self);
    check_success(vector_drop(&extra->content));
    check_success(string_id_map_drop(&extra->inverted_index));
    check_success(vector_drop(&extra->content_freelist));
    free (self->extra);
    self->extra = NULL;
    return STATUS_OK;
}

static int naive_insert(struct string_pool *self, struct string *out, size_t *num_out,
        const char **strings, size_t num_strings)
{
    unused(out);
    unused(num_out);
    unused(strings);
    unused(num_strings);


    naive_lock(self);
    naive_unlock(self);
    return STATUS_NOTIMPL;
}

static int naive_remove(struct string_pool *self, struct string *strings, size_t num_strings)
{
    unused(self);
    unused(strings);
    unused(num_strings);

    naive_lock(self);
    naive_unlock(self);
    return STATUS_NOTIMPL;
}

static int naive_find_by_string(struct string_pool *self, struct string *out, size_t *num_out,
        const char **strings, size_t num_strings)
{
    unused(self);
    unused(out);
    unused(num_out);
    unused(strings);
    unused(num_strings);

    naive_lock(self);
    naive_unlock(self);
    return STATUS_NOTIMPL;
}

static int naive_find_by_id(struct string_pool *self, const char **strings, size_t *num_out,
        const string_id_t *ids, size_t num_ids)
{
    unused(self);
    unused(strings);
    unused(num_out);
    unused(ids);
    unused(num_ids);

    naive_lock(self);
    naive_unlock(self);
    return STATUS_NOTIMPL;
}

static int naive_resolve(struct string_pool *self, char **out, size_t *num_out, struct string *strings,
        size_t num_strings)
{
    unused(self);
    unused(out);
    unused(num_out);
    unused(strings);
    unused(num_strings);

    naive_lock(self);
    naive_unlock(self);
    return STATUS_NOTIMPL;
}

static inline struct naive_extra_t *naive_extra(struct string_pool *self)
{
    assert(self->tag == SP_NAIVE);
    return (struct naive_extra_t *) self->extra;
}

static void naive_lock(struct string_pool *self)
{
    assert(self->tag == SP_NAIVE);
    struct naive_extra_t *extra = naive_extra(self);
    spinlock_lock(&extra->spinlock);
}

static void naive_unlock(struct string_pool *self)
{
    assert(self->tag == SP_NAIVE);
    struct naive_extra_t *extra = naive_extra(self);
    spinlock_unlock(&extra->spinlock);
}

static void naive_setup(struct string_pool* self)
{
    char *empty_str = NULL;
    struct naive_extra_t *extra = naive_extra(self);

    for (string_id_t id = 0; id < extra->content.cap_elems; id++) {
        vector_push(&extra->content, (void *) &empty_str, 1);
        vector_push(&extra->content_freelist, &id, 1);
    }
}