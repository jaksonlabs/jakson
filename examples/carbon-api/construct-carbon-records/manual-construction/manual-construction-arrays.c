// bin/examples-manual-construction-arrays

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    carbon_new context;
    carbon record;
    carbon_insert *ins, *nested_ins;
    carbon_insert_array_state state;
    char *as_json;

    ins = carbon_create_begin(&context, &record, CARBON_KEY_NOKEY, CARBON_KEEP);

    carbon_insert_string(ins, "Hello");
    nested_ins = carbon_insert_array_begin(&state, ins, 1024);
        carbon_insert_string(nested_ins, "you");
        carbon_insert_string(nested_ins, "nested") ;
    carbon_insert_array_end(&state);
    carbon_insert_string(ins, "array!");

    carbon_create_end(&context);

    as_json = carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    carbon_drop(&record);
    free(as_json);

    return 0;
}