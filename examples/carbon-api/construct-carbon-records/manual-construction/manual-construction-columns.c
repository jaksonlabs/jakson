// bin/examples-manual-construction-columns

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    jak_carbon_new context;
    jak_carbon record;
    jak_carbon_insert *ins, *nested_ins;
    jak_carbon_insert_column_state state;
    char *as_json;

    ins = jak_carbon_create_begin(&context, &record, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

    nested_ins = jak_carbon_insert_column_begin(&state, ins, JAK_CARBON_COLUMN_TYPE_U32, 1024);
        jak_carbon_insert_u32(nested_ins, 23);
        jak_carbon_insert_null(nested_ins);
        jak_carbon_insert_u32(nested_ins, 42);
    jak_carbon_insert_column_end(&state);

    jak_carbon_create_end(&context);

    as_json = jak_carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    jak_carbon_drop(&record);
    free(as_json);

    return 0;
}