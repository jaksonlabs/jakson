// bin/examples-manual-construction-arrays

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    jak_carbon_new context;
    jak_carbon record;
    jak_carbon_insert *ins, *nested_ins;
    jak_carbon_insert_array_state state;
    char *as_json;

    ins = jak_carbon_create_begin(&context, &record, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

    jak_carbon_insert_string(ins, "Hello");
    nested_ins = jak_carbon_insert_array_begin(&state, ins, 1024);
        jak_carbon_insert_string(nested_ins, "you");
        jak_carbon_insert_string(nested_ins, "nested") ;
    jak_carbon_insert_array_end(&state);
    jak_carbon_insert_string(ins, "array!");

    jak_carbon_create_end(&context);

    as_json = jak_carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    jak_carbon_drop(&record);
    free(as_json);

    return 0;
}