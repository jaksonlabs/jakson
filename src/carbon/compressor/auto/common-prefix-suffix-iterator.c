#include <carbon/compressor/auto/common-prefix-suffix-iterator.h>
#include <carbon/compressor/compressor-utils.h>

static size_t min(size_t a, size_t b) { return a < b ? a : b; }

carbon_common_prefix_suffix_it carbon_cps_begin(
    carbon_vec_t ofType(char *) *strings,
    carbon_common_prefix_suffix_it_sort_dir_e sort,
    carbon_common_prefix_suffix_it_comp_dir_e comp
) {
    carbon_common_prefix_suffix_it it;
    it.__internals.sort = sort;
    it.__internals.comp = comp;
    it.__internals.previous_string = "";
    it.__internals.previous_length = 0;
    it.__internals.index = -1;
    it.valid = true;

    it.current = "";
    it.current_length = 0;

    carbon_vec_create(&it.__internals.strings, NULL, sizeof(char *), strings->num_elems);
    carbon_vec_cpy_to(&it.__internals.strings, strings);

    qsort(
        it.__internals.strings.base, it.__internals.strings.num_elems, sizeof(char *),
        sort == carbon_cps_sort_from_start ? &carbon_sort_cmp_fwd : &carbon_sort_cmp_rwd
    );

    carbon_cps_next(&it);
    return it;
}

void carbon_cps_next(
    carbon_common_prefix_suffix_it *it
) {
    it->__internals.index++;
    it->index = (size_t)it->__internals.index;

    // We've reached the end
    if(it->index >= it->__internals.strings.num_elems) {
        carbon_vec_drop(&it->__internals.strings);
        it->valid = false;
        return;
    }

    it->__internals.previous_length = it->current_length;
    it->__internals.previous_string = it->current;
    it->current = *(char const * const *)carbon_vec_at(&it->__internals.strings, it->index);
    it->current_length = strlen(it->current);

    size_t max_length    = min(255, min(it->__internals.previous_length, it->current_length));
    size_t common_length = 0;
    if(it->__internals.comp == carbon_cps_comp_from_start) {
        for(; common_length < max_length && it->__internals.previous_string[common_length] == it->current[common_length]; ++common_length);
    } else {
        for(; common_length < max_length && it->__internals.previous_string[it->__internals.previous_length - 1 - common_length] == it->current[it->current_length - 1 - common_length]; ++common_length);
    }

    it->common_length = common_length;
}
