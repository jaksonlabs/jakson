// bin/examples-manual-construction

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    jak_carbon_new context;
    jak_carbon record;
    char *as_json;

    jak_carbon_create_begin(&context, &record, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);
    jak_carbon_create_end(&context);

    as_json = jak_carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    jak_carbon_drop(&record);
    free(as_json);

    return 0;
}