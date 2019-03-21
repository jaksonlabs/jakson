//
// Created by Marcus Pinnecke on 01.03.19.
//

#include "ops-get-citations.h"

CARBON_EXPORT(bool)
ops_get_citations(carbon_timestamp_t *duration, carbon_vec_t ofType(ops_show_values_result_t) *result, const char *paper_title,
                  carbon_archive_t *archive)
{
    CARBON_UNUSED(duration);
    CARBON_UNUSED(result);
    CARBON_UNUSED(paper_title);
    CARBON_UNUSED(archive);


    return true;
}