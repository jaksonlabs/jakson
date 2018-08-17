#include <ng5/storage_engine.h>
#include <ng5/roadfire/roadfire.h>
#include <stdx/string_dics/string_dic_naive.h>
#include <stdx/async.h>

// ---------------------------------------------------------------------------------------------------------------------
//
//  D E F A U L T   C O N F IG
//
// ---------------------------------------------------------------------------------------------------------------------

static struct roadfire_conf roadfire_conf_default = {
    .string_dic_capacity                 = 5000000,
    .string_dic_lookup_num_buckets       =  150000,
    .string_dic_bucket_capacity          =      10,
    .string_dic_nthreads                 =       8,
    .result_register_reserve_num_handles =    1000
};

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

struct roadfire_extra
{
  /* user-empowered configuration */
  struct roadfire_conf                                    conf;

  /* string dictionary to manage string encoding */
  struct string_dic                                       dictionary;

  /* register for results of requests of different kind; memory is recycled if reasonable */
  struct slot_vector of_type (struct vector *)            result_register;

  /* spinlock to ensure thread-safeness */
  struct spinlock                                         spinlock;
};

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define extra_get(engine)                       \
({                                              \
    (struct roadfire_extra *) (engine->extra);  \
})

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

void extra_create(struct storage_engine *roadfire, struct roadfire_conf *conf);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

int storage_engine_roadfire_create(struct storage_engine *roadfire, optional struct roadfire_conf *conf,
        const struct allocator *alloc)
{
    check_non_null(roadfire);
    check_success(allocator_this_or_default(&roadfire->alloc, alloc));
    roadfire->tag = STORAGE_ENGINE_TAG_ROADFIRE;
    extra_create(roadfire, conf);
}

int roadfire_import_strings(struct storage_engine *self,
        optional struct result_handle of_type(store_string_id_t) *out,
        const struct vector_cursor of_type(char *) *strings)
{

}

int roadfire_locate_strings(struct storage_engine *self, optional struct result_handle of_type(store_string_id_t) *out,
        const struct vector_cursor of_type(char *) *strings)
{

}

int roadfire_extract_strings(struct storage_engine *self, struct result_handle of_type(char *) strings,
        const struct vector_cursor of_type(struct compressed_string) *input)
{

}

int roadfire_find_strings(struct storage_engine *self, struct result_handle of_type(store_string_id_t) *out,
        pred_func_t pred)
{

}

int roadfire_drop_result(struct storage_engine *self, slot_vector_slot_t result_slot)
{

}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

void extra_create(struct storage_engine *roadfire, struct roadfire_conf *conf)
{
    assert(out);

    roadfire->extra = allocator_malloc(&roadfire->alloc, (sizeof(struct roadfire_extra)));
    struct roadfire_extra *extra = extra_get(roadfire);
    extra->conf = conf ? *conf : roadfire_conf_default;

    string_dic_create_naive(&extra->dictionary, extra->conf.string_dic_capacity,
            extra->conf.string_dic_lookup_num_buckets, extra->conf.string_dic_bucket_capacity,
            extra->conf.string_dic_nthreads, &roadfire->alloc);

    slot_vector_create(&extra->result_register, &roadfire->alloc, sizeof(struct vector *),
            extra->conf.string_dic_lookup_num_buckets);

    spinlock_create(&extra->spinlock);
}