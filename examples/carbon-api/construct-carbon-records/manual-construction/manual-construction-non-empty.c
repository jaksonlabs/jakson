// bin/examples-manual-construction-non-empty

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    jak_carbon_new context;
    jak_carbon record;
    jak_carbon_insert *ins;
    char *as_json;

    ins = jak_carbon_create_begin(&context, &record, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

    jak_carbon_insert_string(ins, "Hello, Carbon!");
    jak_carbon_insert_unsigned(ins, 42);
    jak_carbon_insert_null(ins);

    jak_carbon_create_end(&context);

    as_json = jak_carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    jak_carbon_drop(&record);
    free(as_json);

    return 0;
}