#include <carbon/carbon-int-archive.h>
#include "carbon/carbon.h"

#include "modules.h"

#include "cli.h"
#include "ops/ops-show-keys.h"
#include "ops/ops-count-values.h"
#include "ops/ops-show-values.h"

#ifdef __GNUC__
#ifdef __linux__
#undef PRIi64
#define PRIi64 "lld"
#endif
#endif

static int testFileExists(FILE *file, const char *fileName, size_t fileNum, size_t fileMax, bool requireExistence)
{
    CARBON_UNUSED(fileNum);
    CARBON_UNUSED(fileMax);

    if (access( fileName, F_OK ) == 0) {
        if (requireExistence) {
            goto success;
        } else {
            goto fail;
        }
    } else {
        if (requireExistence) {
            goto fail;
        } else {
            goto success;
        }
    }

    fail:
    CARBON_CONSOLE_WRITELN(file, "** ERROR ** file I/O error for file '%s'", fileName);
    return false;

    success:
    return true;

}

typedef enum {
    COMMAND_COUNT_ANY
} command_type_e;

typedef struct
{
    command_type_e command_type;

    struct {

    } command_any;
} capture_t;

static void
visit_root_object(carbon_archive_t *archive, carbon_object_id_t id, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
}

static void
before_visit_starts(carbon_archive_t *archive, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(capture);
}

static void
after_visit_ends(carbon_archive_t *archive, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(capture);
}

static carbon_visitor_policy_e
before_object_visit(carbon_archive_t *archive, path_stack_t path,
                                               carbon_object_id_t parent_id, carbon_object_id_t value_id,
                                               uint32_t object_idx, uint32_t num_objects, carbon_string_id_t key,
                                               void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(value_id);
    CARBON_UNUSED(object_idx);
    CARBON_UNUSED(num_objects);
    CARBON_UNUSED(key);
    CARBON_UNUSED(capture);

    // TODO: ???   crashes??
    
    carbon_archive_visitor_print_path(stderr, archive, path);

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
after_object_visit(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                           uint32_t object_idx, uint32_t num_objects, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(object_idx);
    CARBON_UNUSED(num_objects);
    CARBON_UNUSED(capture);

    
}

static void first_prop_type_group(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                              carbon_basic_type_e type, bool is_array, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(type);
    CARBON_UNUSED(is_array);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
}

static void next_prop_type_group(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                             carbon_basic_type_e type, bool is_array, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(type);
    CARBON_UNUSED(is_array);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
}

static void
visit_int8_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              const carbon_string_id_t *keys, const carbon_int8_t *values, uint32_t num_pairs,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);

    
}

static void
visit_int16_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                  const carbon_string_id_t *keys, const carbon_int16_t *values, uint32_t num_pairs,
                  void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);

    
}

static void
visit_int32_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                   const carbon_string_id_t *keys, const carbon_int32_t *values, uint32_t num_pairs,
                   void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);

    


}

static void
visit_int64_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                   const carbon_string_id_t *keys, const carbon_int64_t *values, uint32_t num_pairs,
                   void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);

    
}

static void
visit_uint8_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                  const carbon_string_id_t *keys, const carbon_uint8_t *values, uint32_t num_pairs,
                  void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);

    
}

static void
visit_uint16_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                   const carbon_string_id_t *keys, const carbon_uint16_t *values, uint32_t num_pairs,
                   void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);

    
}

static void
visit_uint32_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                   const carbon_string_id_t *keys, const carbon_uint32_t *values, uint32_t num_pairs,
                   void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);

    
}

static void
visit_uint64_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                   const carbon_string_id_t *keys, const carbon_uint64_t *values, uint32_t num_pairs,
                   void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);

    
}

static void
visit_number_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                   const carbon_string_id_t *keys, const carbon_number_t *values, uint32_t num_pairs,
                   void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);

    
}

static void
visit_string_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                    const carbon_string_id_t *keys, const carbon_string_id_t *values, uint32_t num_pairs,
                    void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);


    
}

static void
visit_boolean_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                    const carbon_string_id_t *keys, const carbon_boolean_t *values, uint32_t num_pairs,
                    void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(values);

    
}

static void visit_null_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                          uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    
}





static carbon_visitor_policy_e
visit_enter_int8_array_pairs(carbon_archive_t *archive, path_stack_t path,
                             carbon_object_id_t id, const carbon_string_id_t *keys,
                             uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_int8_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                        carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                                        void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);
    CARBON_UNUSED(capture);

     
}

static void
visit_int8_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                   carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                                   const carbon_int8_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);
    CARBON_UNUSED(capture);

     
}

static void
visit_leave_int8_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                        uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

     
}

static void
visit_leave_int8_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                         void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     
}



static carbon_visitor_policy_e
visit_enter_int16_array_pairs(carbon_archive_t *archive, path_stack_t path,
                              carbon_object_id_t id, const carbon_string_id_t *keys,
                              uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_int16_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                             carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                             void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     
}

static void
visit_int16_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                        carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                        const carbon_int16_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);

     
}

static void
visit_leave_int16_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                             uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     
}

static void
visit_leave_int16_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     
}



static carbon_visitor_policy_e
visit_enter_int32_array_pairs(carbon_archive_t *archive, path_stack_t path,
                              carbon_object_id_t id, const carbon_string_id_t *keys,
                              uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_int32_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                             carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                             void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     
}

static void
visit_int32_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                        carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                        const carbon_int32_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);

     
}

static void
visit_leave_int32_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                             uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     
}

static void
visit_leave_int32_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     
}



static carbon_visitor_policy_e
visit_enter_int64_array_pairs(carbon_archive_t *archive, path_stack_t path,
                              carbon_object_id_t id, const carbon_string_id_t *keys,
                              uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_int64_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                             carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                             void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     

}

static void
visit_int64_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                        carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                        const carbon_int64_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);

     
}

static void
visit_leave_int64_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                             uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     

}

static void
visit_leave_int64_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     
}



static carbon_visitor_policy_e
visit_enter_uint8_array_pairs(carbon_archive_t *archive, path_stack_t path,
                              carbon_object_id_t id, const carbon_string_id_t *keys,
                              uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(keys);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_uint8_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                             carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                             void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     

}

static void
visit_uint8_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                        carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                        const carbon_uint8_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);

    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);

     
}

static void
visit_leave_uint8_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                             uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     
}

static void
visit_leave_uint8_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     
}



static carbon_visitor_policy_e
visit_enter_uint16_array_pairs(carbon_archive_t *archive, path_stack_t path,
                               carbon_object_id_t id, const carbon_string_id_t *keys,
                               uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_uint16_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     
}

static void
visit_uint16_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                         carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                         const carbon_uint16_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);

     

}

static void
visit_leave_uint16_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     
}

static void
visit_leave_uint16_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                               void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     
}



static carbon_visitor_policy_e
visit_enter_uint32_array_pairs(carbon_archive_t *archive, path_stack_t path,
                               carbon_object_id_t id, const carbon_string_id_t *keys,
                               uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_uint32_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     
}

static void
visit_uint32_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                         carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                         const carbon_uint32_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);

     
}

static void
visit_leave_uint32_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     
}

static void
visit_leave_uint32_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                               void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     
}



static carbon_visitor_policy_e
visit_enter_uint64_array_pairs(carbon_archive_t *archive, path_stack_t path,
                               carbon_object_id_t id, const carbon_string_id_t *keys,
                               uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_uint64_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     
}

static void
visit_uint64_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                         carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                         const carbon_uint64_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);

     
}

static void
visit_leave_uint64_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     
}

static void
visit_leave_uint64_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                               void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     
}



static carbon_visitor_policy_e
visit_enter_number_array_pairs(carbon_archive_t *archive, path_stack_t path,
                               carbon_object_id_t id, const carbon_string_id_t *keys,
                               uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_number_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     
}

static void
visit_number_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                         carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                         const carbon_number_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);

     
}

static void
visit_leave_number_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     
}

static void
visit_leave_number_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                               void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     

}



static carbon_visitor_policy_e
visit_enter_string_array_pairs(carbon_archive_t *archive, path_stack_t path,
                               carbon_object_id_t id, const carbon_string_id_t *keys,
                               uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_string_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     
}

static void
visit_string_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                         carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                         const carbon_string_id_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);

    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);

     
}

static void
visit_leave_string_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     
}

static void
visit_leave_string_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                               void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     

}



static carbon_visitor_policy_e
visit_enter_boolean_array_pairs(carbon_archive_t *archive, path_stack_t path,
                                carbon_object_id_t id, const carbon_string_id_t *keys,
                                uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_enter_boolean_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                               carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,
                               void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     

}

static void
visit_boolean_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                          carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                          const carbon_boolean_t *array, uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);

    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);

     

}

static void
visit_leave_boolean_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                               uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     

}

static void
visit_leave_boolean_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     

}

static carbon_visitor_policy_e
visit_enter_null_array_pairs(carbon_archive_t *archive, path_stack_t path,
                                                        carbon_object_id_t id,
                                                        const carbon_string_id_t *keys, uint32_t num_pairs,
                                                        void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);

     

    return CARBON_VISITOR_POLICY_INCLUDE;

}

static void
visit_enter_null_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                    carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);

     
}

static void
visit_null_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                               carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                               carbon_uint32_t num_nulls, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);

    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(num_nulls);

     
}

static void
visit_leave_null_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                    uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);

     


}

static void
visit_leave_null_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                     void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(capture);

     

}

static carbon_visitor_policy_e
before_visit_object_array(carbon_archive_t *archive, path_stack_t path,
                                                     carbon_object_id_t parent_id, carbon_string_id_t key,
                                                     void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);

    carbon_archive_visitor_print_path(stderr, archive, path);
     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
before_visit_object_array_objects(bool *skip_group_object_ids,
                                          carbon_archive_t *archive, path_stack_t path,
                                          carbon_object_id_t parent_id,
                                          carbon_string_id_t key,
                                          const carbon_object_id_t *group_object_ids,
                                          uint32_t num_group_object_ids, void *capture)
{
    CARBON_UNUSED(skip_group_object_ids);
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(group_object_ids);
    CARBON_UNUSED(num_group_object_ids);

     

}

static carbon_visitor_policy_e
before_visit_object_array_object_property(carbon_archive_t *archive, path_stack_t path,
                                                                     carbon_object_id_t parent_id,
                                                                     carbon_string_id_t key,
                                                                     carbon_string_id_t nested_key,
                                                                     carbon_basic_type_e nested_value_type,
                                                                     void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);
    CARBON_UNUSED(nested_value_type);

     

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_object_array_object_property_int8(carbon_archive_t *archive, path_stack_t path,
                                               carbon_object_id_t parent_id,
                                               carbon_string_id_t key,
                                               carbon_object_id_t nested_object_id,
                                               carbon_string_id_t nested_key,
                                               const carbon_int8_t *nested_values,
                                               uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}

static void
visit_object_array_object_property_int16(carbon_archive_t *archive, path_stack_t path,
                                        carbon_object_id_t parent_id,
                                        carbon_string_id_t key,
                                        carbon_object_id_t nested_object_id,
                                        carbon_string_id_t nested_key,
                                        const carbon_int16_t *nested_values,
                                        uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}

static void
visit_object_array_object_property_int32(carbon_archive_t *archive, path_stack_t path,
                                         carbon_object_id_t parent_id,
                                         carbon_string_id_t key,
                                         carbon_object_id_t nested_object_id,
                                         carbon_string_id_t nested_key,
                                         const carbon_int32_t *nested_values,
                                         uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}


static void
visit_object_array_object_property_int64(carbon_archive_t *archive, path_stack_t path,
                                         carbon_object_id_t parent_id,
                                         carbon_string_id_t key,
                                         carbon_object_id_t nested_object_id,
                                         carbon_string_id_t nested_key,
                                         const carbon_int64_t *nested_values,
                                         uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}



static void
visit_object_array_object_property_uint8(carbon_archive_t *archive, path_stack_t path,
                                         carbon_object_id_t parent_id,
                                         carbon_string_id_t key,
                                         carbon_object_id_t nested_object_id,
                                         carbon_string_id_t nested_key,
                                         const carbon_uint8_t *nested_values,
                                         uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}


static void
visit_object_array_object_property_uint16(carbon_archive_t *archive, path_stack_t path,
                                        carbon_object_id_t parent_id,
                                        carbon_string_id_t key,
                                        carbon_object_id_t nested_object_id,
                                        carbon_string_id_t nested_key,
                                        const carbon_uint16_t *nested_values,
                                        uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}


static void
visit_object_array_object_property_uint32(carbon_archive_t *archive, path_stack_t path,
                                         carbon_object_id_t parent_id,
                                         carbon_string_id_t key,
                                         carbon_object_id_t nested_object_id,
                                         carbon_string_id_t nested_key,
                                         const carbon_uint32_t *nested_values,
                                         uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);
    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}


static void
visit_object_array_object_property_uint64(carbon_archive_t *archive, path_stack_t path,
                                         carbon_object_id_t parent_id,
                                         carbon_string_id_t key,
                                         carbon_object_id_t nested_object_id,
                                         carbon_string_id_t nested_key,
                                         const carbon_uint64_t *nested_values,
                                         uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}


static void
visit_object_array_object_property_numbers(carbon_archive_t *archive, path_stack_t path,
                                         carbon_object_id_t parent_id,
                                         carbon_string_id_t key,
                                         carbon_object_id_t nested_object_id,
                                         carbon_string_id_t nested_key,
                                         const carbon_number_t *nested_values,
                                         uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}


static void
visit_object_array_object_property_strings(carbon_archive_t *archive, path_stack_t path,
                                          carbon_object_id_t parent_id,
                                          carbon_string_id_t key,
                                          carbon_object_id_t nested_object_id,
                                          carbon_string_id_t nested_key,
                                          const carbon_string_id_t *nested_values,
                                          uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}


static void
visit_object_array_object_property_booleans(carbon_archive_t *archive, path_stack_t path,
                                          carbon_object_id_t parent_id,
                                          carbon_string_id_t key,
                                          carbon_object_id_t nested_object_id,
                                          carbon_string_id_t nested_key,
                                          const carbon_boolean_t *nested_values,
                                          uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}

static void
visit_object_array_object_property_nulls(carbon_archive_t *archive, path_stack_t path,
                                           carbon_object_id_t parent_id,
                                           carbon_string_id_t key,
                                           carbon_object_id_t nested_object_id,
                                           carbon_string_id_t nested_key,
                                           const carbon_uint32_t *nested_values,
                                           uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);

     
}

static carbon_visitor_policy_e
before_object_array_object_property_object(carbon_archive_t *archive, path_stack_t path,
                                                                      carbon_object_id_t parent_id,
                                                                      carbon_string_id_t key,
                                                                      carbon_object_id_t nested_object_id,
                                                                      carbon_string_id_t nested_key,
                                                                      uint32_t nested_value_object_id,
                                                                      void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);

    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_value_object_id);

     carbon_archive_visitor_print_path(stdout, archive, path);

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
run_magic_visitor(int mask, carbon_archive_t *archive)
{
    carbon_archive_visitor_t visitor = { 0 };
    carbon_archive_visitor_desc_t desc = { .visit_mask = mask };
    capture_t capture = {

    };

    visitor.visit_root_object   = visit_root_object;
    visitor.before_object_visit = before_object_visit;
    visitor.visit_int8_pairs    = visit_int8_pairs;
    visitor.visit_int16_pairs   = visit_int16_pairs;
    visitor.visit_int32_pairs   = visit_int32_pairs;
    visitor.visit_int64_pairs   = visit_int64_pairs;
    visitor.visit_uint8_pairs   = visit_uint8_pairs;
    visitor.visit_uint16_pairs  = visit_uint16_pairs;
    visitor.visit_uint32_pairs  = visit_uint32_pairs;
    visitor.visit_uint64_pairs  = visit_uint64_pairs;
    visitor.visit_number_pairs  = visit_number_pairs;
    visitor.visit_string_pairs  = visit_string_pairs;
    visitor.visit_boolean_pairs = visit_boolean_pairs;
    visitor.visit_null_pairs    = visit_null_pairs;

    visitor.visit_enter_int8_array_pairs = visit_enter_int8_array_pairs;
    visitor.visit_int8_array_pair = visit_int8_array_pair;
    visitor.visit_enter_int16_array_pairs = visit_enter_int16_array_pairs;
    visitor.visit_int16_array_pair = visit_int16_array_pair;
    visitor.visit_enter_int32_array_pairs = visit_enter_int32_array_pairs;
    visitor.visit_int32_array_pair = visit_int32_array_pair;
    visitor.visit_enter_int64_array_pairs = visit_enter_int64_array_pairs;
    visitor.visit_int64_array_pair = visit_int64_array_pair;
    visitor.visit_enter_uint8_array_pairs = visit_enter_uint8_array_pairs;
    visitor.visit_uint8_array_pair = visit_uint8_array_pair;
    visitor.visit_enter_uint16_array_pairs = visit_enter_uint16_array_pairs;
    visitor.visit_uint16_array_pair = visit_uint16_array_pair;
    visitor.visit_enter_uint32_array_pairs = visit_enter_uint32_array_pairs;
    visitor.visit_uint32_array_pair = visit_uint32_array_pair;
    visitor.visit_enter_uint64_array_pairs = visit_enter_uint64_array_pairs;
    visitor.visit_uint64_array_pair = visit_uint64_array_pair;
    visitor.visit_enter_boolean_array_pairs = visit_enter_boolean_array_pairs;
    visitor.visit_boolean_array_pair = visit_boolean_array_pair;
    visitor.visit_enter_number_array_pairs = visit_enter_number_array_pairs;
    visitor.visit_number_array_pair = visit_number_array_pair;
    visitor.visit_enter_null_array_pairs = visit_enter_null_array_pairs;
    visitor.visit_null_array_pair = visit_null_array_pair;
    visitor.visit_enter_string_array_pairs = visit_enter_string_array_pairs;
    visitor.visit_string_array_pair = visit_string_array_pair;

    visitor.before_visit_object_array_objects = before_visit_object_array_objects;

    visitor.visit_object_array_object_property_int8s = visit_object_array_object_property_int8;
    visitor.visit_object_array_object_property_int16s = visit_object_array_object_property_int16;
    visitor.visit_object_array_object_property_int32s = visit_object_array_object_property_int32;
    visitor.visit_object_array_object_property_int64s = visit_object_array_object_property_int64;
    visitor.visit_object_array_object_property_uint8s = visit_object_array_object_property_uint8;
    visitor.visit_object_array_object_property_uint16s = visit_object_array_object_property_uint16;
    visitor.visit_object_array_object_property_uint32s = visit_object_array_object_property_uint32;
    visitor.visit_object_array_object_property_uint64s = visit_object_array_object_property_uint64;
    visitor.visit_object_array_object_property_numbers = visit_object_array_object_property_numbers;
    visitor.visit_object_array_object_property_strings = visit_object_array_object_property_strings;
    visitor.visit_object_array_object_property_booleans = visit_object_array_object_property_booleans;
    visitor.visit_object_array_object_property_nulls = visit_object_array_object_property_nulls;

    visitor.before_visit_starts = before_visit_starts;
    visitor.after_visit_ends = after_visit_ends;
    visitor.after_object_visit = after_object_visit;
    visitor.first_prop_type_group = first_prop_type_group;
    visitor.next_prop_type_group = next_prop_type_group;
    visitor.visit_enter_int8_array_pair = visit_enter_int8_array_pair;
    visitor.visit_leave_int8_array_pair = visit_leave_int8_array_pair;
    visitor.visit_leave_int8_array_pairs = visit_leave_int8_array_pairs;
    visitor.visit_enter_int16_array_pair = visit_enter_int16_array_pair;
    visitor.visit_leave_int16_array_pair = visit_leave_int16_array_pair;
    visitor.visit_leave_int16_array_pairs = visit_leave_int16_array_pairs;
    visitor.visit_enter_int32_array_pair = visit_enter_int32_array_pair;
    visitor.visit_leave_int32_array_pair = visit_leave_int32_array_pair;
    visitor.visit_leave_int32_array_pairs = visit_leave_int32_array_pairs;
    visitor.visit_enter_int64_array_pair = visit_enter_int64_array_pair;
    visitor.visit_leave_int64_array_pair = visit_leave_int64_array_pair;
    visitor.visit_leave_int64_array_pairs = visit_leave_int64_array_pairs;
    visitor.visit_enter_uint8_array_pair = visit_enter_uint8_array_pair;
    visitor.visit_leave_uint8_array_pair = visit_leave_uint8_array_pair;
    visitor.visit_leave_uint8_array_pairs = visit_leave_uint8_array_pairs;
    visitor.visit_enter_uint16_array_pair = visit_enter_uint16_array_pair;
    visitor.visit_leave_uint16_array_pair = visit_leave_uint16_array_pair;
    visitor.visit_leave_uint16_array_pairs = visit_leave_uint16_array_pairs;
    visitor.visit_enter_uint32_array_pair = visit_enter_uint32_array_pair;
    visitor.visit_leave_uint32_array_pair = visit_leave_uint32_array_pair;
    visitor.visit_leave_uint32_array_pairs = visit_leave_uint32_array_pairs;
    visitor.visit_enter_uint64_array_pair = visit_enter_uint64_array_pair;
    visitor.visit_leave_uint64_array_pair = visit_leave_uint64_array_pair;
    visitor.visit_leave_uint64_array_pairs = visit_leave_uint64_array_pairs;
    visitor.visit_enter_number_array_pair = visit_enter_number_array_pair;
    visitor.visit_leave_number_array_pair = visit_leave_number_array_pair;
    visitor.visit_leave_number_array_pairs = visit_leave_number_array_pairs;
    visitor.visit_enter_string_array_pair = visit_enter_string_array_pair;
    visitor.visit_leave_string_array_pair = visit_leave_string_array_pair;
    visitor.visit_leave_string_array_pairs = visit_leave_string_array_pairs;
    visitor.visit_enter_boolean_array_pair = visit_enter_boolean_array_pair;
    visitor.visit_leave_boolean_array_pair = visit_leave_boolean_array_pair;
    visitor.visit_leave_boolean_array_pairs = visit_leave_boolean_array_pairs;
    visitor.visit_enter_null_array_pair = visit_enter_null_array_pair;
    visitor.visit_leave_null_array_pair = visit_leave_null_array_pair;
    visitor.visit_leave_null_array_pairs = visit_leave_null_array_pairs;
    visitor.before_visit_object_array = before_visit_object_array;
    visitor.before_visit_object_array_object_property = before_visit_object_array_object_property;
    visitor.before_object_array_object_property_object = before_object_array_object_property_object;

    carbon_archive_visit_archive(archive, &desc, &visitor, &capture);
}

static bool
run_show_keys( carbon_timestamp_t *duration, carbon_encoded_doc_collection_t *result, const char *path, carbon_archive_t *archive)
{
    carbon_vec_t ofType(ops_show_keys_key_type_pair_t) prop_keys;
    carbon_vec_create(&prop_keys, NULL, sizeof(ops_show_keys_key_type_pair_t), 100);
    carbon_object_id_t result_oid;
    carbon_object_id_create(&result_oid);

    ops_show_keys(duration, &prop_keys, path, archive);

    carbon_encoded_doc_collection_create(result, &archive->err, archive);

    carbon_encoded_doc_t *result_doc = encoded_doc_collection_get_or_append(result, result_oid);
    carbon_encoded_doc_add_prop_array_object_decoded(result_doc, "result");

    for (uint32_t i = 0; i < prop_keys.num_elems; i++) {
        ops_show_keys_key_type_pair_t *pair = CARBON_VECTOR_GET(&prop_keys, i, ops_show_keys_key_type_pair_t);
        carbon_object_id_t tmp_obj_id;
        carbon_object_id_create(&tmp_obj_id);
        carbon_encoded_doc_array_push_object_decoded(result_doc, "result", tmp_obj_id);
        carbon_encoded_doc_t *pair_doc = encoded_doc_collection_get_or_append(result, tmp_obj_id);
        carbon_encoded_doc_add_prop_string_decoded(pair_doc, "name", pair->key);
        carbon_encoded_doc_add_prop_string_decoded_string_value_decoded(pair_doc, "type", carbon_basic_type_to_json_type_str(pair->type));
        carbon_encoded_doc_add_prop_string_decoded_string_value_decoded(pair_doc, "sub-type", carbon_basic_type_to_system_type_str(pair->type));
    }


    carbon_vec_drop(&prop_keys);
    return true;
}

static bool
run_count_values( carbon_timestamp_t *duration, carbon_encoded_doc_collection_t *result, const char *path, carbon_archive_t *archive)
{
    carbon_vec_t ofType(ops_count_values_result_t) prop_keys;
    carbon_vec_create(&prop_keys, NULL, sizeof(ops_count_values_result_t), 100);
    carbon_object_id_t result_oid;
    carbon_object_id_create(&result_oid);

    ops_count_values(duration, &prop_keys, path, archive);

    carbon_encoded_doc_collection_create(result, &archive->err, archive);

    carbon_encoded_doc_t *result_doc = encoded_doc_collection_get_or_append(result, result_oid);
    carbon_encoded_doc_add_prop_array_object_decoded(result_doc, "result");

    for (uint32_t i = 0; i < prop_keys.num_elems; i++) {
        ops_count_values_result_t *entry = CARBON_VECTOR_GET(&prop_keys, i, ops_count_values_result_t);
        carbon_object_id_t tmp_obj_id;
        carbon_object_id_create(&tmp_obj_id);
        carbon_encoded_doc_array_push_object_decoded(result_doc, "result", tmp_obj_id);
        carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(result, tmp_obj_id);
        carbon_encoded_doc_add_prop_string_decoded(doc, "key", entry->key);
        carbon_encoded_doc_add_prop_uint32_decoded(doc, "count", entry->count);
    }


    carbon_vec_drop(&prop_keys);
    return true;
}

static bool
run_show_values( carbon_timestamp_t *duration, carbon_encoded_doc_collection_t *result, const char *path, carbon_archive_t *archive, uint32_t limit)
{
    carbon_vec_t ofType(ops_show_values_result_t) prop_keys;
    carbon_vec_create(&prop_keys, NULL, sizeof(ops_show_values_result_t), 100);
    carbon_object_id_t result_oid;
    carbon_object_id_create(&result_oid);

    ops_show_values(duration, &prop_keys, path, archive, limit);

    carbon_encoded_doc_collection_create(result, &archive->err, archive);

    carbon_encoded_doc_t *result_doc = encoded_doc_collection_get_or_append(result, result_oid);
    carbon_encoded_doc_add_prop_array_object_decoded(result_doc, "result");

    for (uint32_t i = 0; i < prop_keys.num_elems; i++) {
        ops_show_values_result_t *entry = CARBON_VECTOR_GET(&prop_keys, i, ops_show_values_result_t);
        carbon_object_id_t tmp_obj_id;
        carbon_object_id_create(&tmp_obj_id);
        carbon_encoded_doc_array_push_object_decoded(result_doc, "result", tmp_obj_id);
        carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(result, tmp_obj_id);
        carbon_encoded_doc_add_prop_string_decoded(doc, "key", entry->key);
        carbon_encoded_doc_add_prop_string_decoded_string_value_decoded(doc, "type", carbon_basic_type_to_system_type_str(entry->type));
    }


    carbon_vec_drop(&prop_keys);
    return true;
}

static void
process_from(carbon_archive_t *archive, const char *line)
{
    carbon_timestamp_t duration = 0;
    char *select = strstr(line, "select");
    char *show_keys = strstr(line, "show keys");
    if (select)
    {
        char *path = strdup(line + 1);
        path[select - line - 2] = '\0';
        char *command = strdup(line + 1 + strlen(path) + 2 + strlen("select"));

        if (strcmp(command, "count(*)") == 0) {
            carbon_encoded_doc_collection_t result;
            carbon_timestamp_t duration;
            run_count_values(&duration, &result, path, archive);
            carbon_encoded_doc_collection_print(stdout, &result);
            carbon_encoded_doc_collection_drop(&result);
            printf("\n");
            printf("execution time: %" PRIi64"ms\n", duration);
        } else if (strstr(command, "*") != 0) {
            int offset_count = 0;
            int limit_count = INT32_MAX;
            if (strstr(command, "* offset ") != 0 || strstr(command, "* limit ") != 0) {
                if (strstr(command, "* offset ")) {
                    char *offset = strstr(command, "offset ") + strlen("offset ");
                    char *blank = NULL;
                    blank = strstr(offset, " ");
                    if (!blank) {
                        blank = offset + strlen(offset);
                    }
                    if (blank) {
                        offset[blank - offset] = '\0';
                        offset_count = atoi(offset);
                    }
                    else {
                        fprintf(stderr, "parsing error for <offset M>: expected <limit N> afterwards");
                        goto leave;
                    }
                }

                if (strstr(select, "limit ") != 0) {
                    char *limit = strstr(select, "limit ") + strlen("limit ");
                    limit_count = atoi(limit);
                }

            }

            printf("offset_count %d\n", offset_count);
            printf("limit %d\n", limit_count);

            carbon_encoded_doc_collection_t result;

            run_show_values(&duration, &result, path, archive, (uint32_t) limit_count);
            carbon_encoded_doc_collection_print(stdout, &result);
            carbon_encoded_doc_collection_drop(&result);
leave:
            printf("\n");
            printf("execution time: %" PRIi64"ms\n", duration);


        }



        free(path);
        free(command);

    } else if (show_keys)
    {
        char *path = strdup(line + 1);
        path[show_keys - line - 2] = '\0';

        carbon_encoded_doc_collection_t result;
        carbon_timestamp_t duration;
        run_show_keys(&duration, &result, path, archive);
        carbon_encoded_doc_collection_print(stdout, &result);
        carbon_encoded_doc_collection_drop(&result);
        printf("\n");
        printf("execution time: %" PRIi64"ms\n", duration);

        free(path);
    } else {
        printf("unexpected token found\n");
    }


}

static bool
process_command(carbon_archive_t *archive)
{
    CARBON_UNUSED(archive);

    fprintf(stdout, "> ");

    char *line = NULL;
    size_t size;
    if (getline(&line, &size, stdin) == -1) {
        fprintf(stderr, "no line\n");
    } else {
        line[strlen(line) - 1] = '\0';

        if (strncmp(line, "from", strlen("from")) == 0) {
            process_from(archive, line + strlen("from"));
        } else if (strcmp(line, ".help") == 0) {
            printf("\nUse one of the following statements:\n"
                       "\tfrom /<path> show keys\t\t\t\t\tto show keys of object(s) behind <path>\n"
                       "\tfrom /<path>/<key> select count(*)\t\tto count values for objects in <path> having key <key> ");
            printf("\n\n");
            printf("Type .examples for examples and .exit to leave this shell.");
            printf("\n\n");
        } else if (strcmp(line, ".examples") == 0) {
            printf("from / show keys\n"
                   "from /authors show keys\n"
                   "from /title select count(*)\n"
                   "from /authors/name select count(*)\n\n");

        } else if (strcmp(line, ".exit") == 0) {
            fprintf(stdout, "%s", "bye");
            return false;
        } else {
            fprintf(stdout, "no such command: %s\n", line);
        }





    }
    return true;
}

bool moduleCliInvoke(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_UNUSED(manager);
    CARBON_UNUSED(argv);
    CARBON_UNUSED(argc);
    CARBON_UNUSED(file);

    if (argc != 1) {
        CARBON_CONSOLE_WRITELN(file, "Run '%s' to see usage.", "$ carbon-tool cli");
        return false;
    } else {
        const int filePathArgIdx = 0;
        const char *pathCarbonFileIn = argv[filePathArgIdx];

        CARBON_CONSOLE_OUTPUT_OFF()
        if (testFileExists(file, pathCarbonFileIn, 1, 1, true) != true) {
            CARBON_CONSOLE_OUTPUT_ON()
            CARBON_CONSOLE_WRITELN(file, "Input file cannot be found. %s", "STOP.");
            return false;
        }
        CARBON_CONSOLE_OUTPUT_ON()

        FILE *f = fopen(pathCarbonFileIn, "r");
        fseek(f, 0, SEEK_END);
        size_t file_size = ftell(f);
        fclose(f);

        carbon_archive_t archive;
        int status;
        if ((status = carbon_archive_open(&archive, pathCarbonFileIn))) {

            carbon_archive_info_t info;
            carbon_archive_get_info(&info, &archive);
            printf("CARBON file successfully loaded: '%s' (%.2f GiB) \n%.2f MiB record data, %.2f MiB string table (%" PRIu32 " strings), %.2f MiB index data\n",
                    pathCarbonFileIn, file_size / 1024.0 / 1024.0 / 1024.0,
                    info.record_table_size / 1024.0 / 1024.0, info.string_table_size / 1024.0 / 1024.0, info.num_embeddded_strings,
                    info.string_id_index_size / 1024.0 / 1024.0);



            printf("Type '.help' for usage instructions.\n\n");

            while (process_command(&archive))
            { };

            carbon_archive_close(&archive);
        } else {
            CARBON_PRINT_ERROR(archive.err.code);
        }



        return true;
    }
}
