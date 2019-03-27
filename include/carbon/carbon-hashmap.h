#ifndef CARBON_HASHMAP_H
#define CARBON_HASHMAP_H

/**
 * Hasmap implementation based on https://github.com/petewarden/c_hashmap
 *
 * From original header comment:
 * > Originally by Elliot C Back - http://elliottback.com/wp/hashmap-implementation-in-c/
 * >
 * > Modified by Pete Warden to fix a serious performance problem, support strings as keys
 * > and removed thread synchronization - http://petewarden.typepad.com
 **/

#include "carbon-common.h"

typedef enum carbon_hashmap_status {
    carbon_hashmap_status_ok = -1,
    carbon_hashmap_status_missing = -2,
    carbon_hashmap_status_full = -3,
    carbon_hashmap_status_out_of_memory = -4
} carbon_hashmap_status_t;

typedef enum carbon_hashmap_iterator_status {
    carbon_hashmap_iterator_status_valid = 1,
    carbon_hashmap_iterator_status_invalid = 0
} carbon_hashmap_iterator_status_t;

typedef void *carbon_hashmap_any_t;
typedef void *carbon_hashmap_t;

/*
 * pointer to a function that can take two any_t arguments
 * and return an integer, returns status code
 */
typedef carbon_hashmap_status_t (*carbon_hashmap_iterator_function_t)(char const*, carbon_hashmap_any_t);

typedef carbon_hashmap_status_t (*carbon_hashmap_iterator_function_with_index_t)(char const*, size_t, carbon_hashmap_any_t, carbon_hashmap_any_t);

typedef struct carbon_hashmap_iterator {
    struct {
        int internal_index;
        carbon_hashmap_t *map;
    } __it;

    size_t index;
    char const * key;
    carbon_hashmap_any_t * value;
    carbon_hashmap_iterator_status_t valid;
} carbon_hashmap_iterator_t;

/*
 * Return an empty hashmap.
*/
carbon_hashmap_t carbon_hashmap_new();


carbon_hashmap_iterator_t carbon_hashmap_begin(carbon_hashmap_t *map);
carbon_hashmap_iterator_status_t carbon_hashmap_next(carbon_hashmap_iterator_t *it);

/*
 * Add an element to the hashmap. Return MAP_OK or MAP_OMEM.
 */
carbon_hashmap_status_t carbon_hashmap_put(
        carbon_hashmap_t map,
        char const* key,
        carbon_hashmap_any_t value
);

/*
 * Get an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
carbon_hashmap_status_t carbon_hashmap_get(
        carbon_hashmap_t map,
        char const* key,
        carbon_hashmap_any_t *arg
);

/*
 * Remove an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
carbon_hashmap_status_t carbon_hashmap_remove(
        carbon_hashmap_t map,
        char* key
);

/*
 * Free the hashmap
 */
void carbon_hashmap_drop(carbon_hashmap_t map);

/*
 * Get the current size of a hashmap
 */
size_t carbon_hashmap_length(carbon_hashmap_t map);

#endif // CARBON_HASHMAP_H
