// bin/examples-manual-construction-columns

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    carbon_new context;
    carbon record;
    carbon_insert *ins, *nested_ins;
    carbon_insert_column_state state;
    char *as_json;

    ins = carbon_create_begin(&context, &record, CARBON_KEY_NOKEY, CARBON_KEEP);

    nested_ins = carbon_insert_column_begin(&state, ins, CARBON_COLUMN_TYPE_U32, 1024);
        carbon_insert_u32(nested_ins, 23);
        carbon_insert_null(nested_ins);
        carbon_insert_u32(nested_ins, 42);
    carbon_insert_column_end(&state);

    carbon_create_end(&context);

    as_json = carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    carbon_drop(&record);
    free(as_json);

    return 0;
}