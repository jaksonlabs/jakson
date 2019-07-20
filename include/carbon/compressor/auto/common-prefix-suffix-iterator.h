#ifndef CARBON_COMMON_PREFIX_SUFFIX_ITERATOR_H
#define CARBON_COMMON_PREFIX_SUFFIX_ITERATOR_H

#include <carbon/carbon-common.h>
#include <carbon/carbon-vector.h>

typedef enum carbon_common_prefix_suffix_it_sort_dir {
    carbon_cps_sort_from_start,
    carbon_cps_sort_from_end
} carbon_common_prefix_suffix_it_sort_dir_e;

typedef enum carbon_common_prefix_suffix_it_cmp_dir {
    carbon_cps_comp_from_start,
    carbon_cps_comp_from_end
} carbon_common_prefix_suffix_it_comp_dir_e;

typedef struct {
    size_t       previous_length;
    char const * previous_string;

    carbon_common_prefix_suffix_it_sort_dir_e sort;
    carbon_common_prefix_suffix_it_comp_dir_e comp;

    ssize_t index;

    carbon_vec_t ofType(char *) strings;
} carbon_common_prefix_suffix_it_internals;

typedef struct {
    carbon_common_prefix_suffix_it_internals __internals;

    size_t       common_length;
    size_t       current_length;
    char const * current;
    size_t       index;

    bool         valid;
} carbon_common_prefix_suffix_it;

typedef carbon_common_prefix_suffix_it carbon_cps_it;

carbon_common_prefix_suffix_it carbon_cps_begin(
    carbon_vec_t ofType(char *) *strings,
    carbon_common_prefix_suffix_it_sort_dir_e sort,
    carbon_common_prefix_suffix_it_comp_dir_e comp
);

void carbon_cps_next(
    carbon_common_prefix_suffix_it *it
);

#endif // CARBON_COMMON_PREFIX_SUFFIX_ITERATOR_H
