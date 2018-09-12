#include <ng5/ng5_storage_engine.h>
#include <ng5/roadfire/ng5_roadfire.h>
#include <stdx/ng5_string_dic_sync.h>
#include <stdx/ng5_spinlock.h>

// ---------------------------------------------------------------------------------------------------------------------
//
//  D E F A U L T   C O N F IG
//
// ---------------------------------------------------------------------------------------------------------------------

struct roadfire_conf roadfire_conf_default = {
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
  struct Dictionary                                       dictionary;

  /* register for results of requests of different kind; memory is recycled if reasonable */
  struct slot_vector of_type (struct ng5_vector *)            result_register;

  /* spinlock to ensure thread-safeness */
  struct ng5_spinlock                                         spinlock;
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
        const ng5_allocator_t *alloc)
{
    check_non_null(roadfire);
    check_success(allocator_this_or_default(&roadfire->alloc, alloc));
    roadfire->tag = STORAGE_ENGINE_TAG_ROADFIRE;
    extra_create(roadfire, conf);
    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

void extra_create(struct storage_engine *roadfire, struct roadfire_conf *conf)
{
    assert(roadfire);

    roadfire->extra = allocator_malloc(&roadfire->alloc, (sizeof(struct roadfire_extra)));
    struct roadfire_extra *extra = extra_get(roadfire);
    extra->conf = conf ? *conf : roadfire_conf_default;

    string_dic_create_sync(&extra->dictionary, extra->conf.string_dic_capacity,
            extra->conf.string_dic_lookup_num_buckets, extra->conf.string_dic_bucket_capacity,
            extra->conf.string_dic_nthreads, &roadfire->alloc);

    slot_vector_create(&extra->result_register, &roadfire->alloc, sizeof(struct ng5_vector *),
            extra->conf.string_dic_lookup_num_buckets);

    ng5_spinlock_create(&extra->spinlock);
}