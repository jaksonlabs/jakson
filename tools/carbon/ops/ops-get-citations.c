//
// Created by Marcus Pinnecke on 01.03.19.
//

#include "ops-get-citations.h"

NG5_EXPORT(bool)
ops_get_citations(carbon_timestamp_t *duration, vec_t ofType(ops_show_values_result_t) *result, const char *paper_title,
                  carbon_archive_t *archive)
{
    NG5_UNUSED(duration);
    NG5_UNUSED(result);
    NG5_UNUSED(paper_title);
    NG5_UNUSED(archive);


    return true;
}