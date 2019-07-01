//
// Created by Marcus Pinnecke on 01.03.19.
//

#include "ops-get-citations.h"

NG5_EXPORT(bool)
ops_get_citations(timestamp_t *duration, struct vector ofType(ops_show_values_result_t) *result, const char *paper_title,
                  struct archive *archive)
{
    unused(duration);
    unused(result);
    unused(paper_title);
    unused(archive);


    return true;
}