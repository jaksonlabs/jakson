// bin/examples-manual-construction-objects

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    jak_carbon_new context;
    jak_carbon record;
    jak_carbon_insert *ins, *nested_ins, *prop_ins;
    jak_carbon_insert_object_state state, as_prop_state;
    char *as_json;

    ins = jak_carbon_create_begin(&context, &record, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

    nested_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_string(nested_ins, "key", "value");
        prop_ins = jak_carbon_insert_prop_object_begin(&as_prop_state, nested_ins, "as prop", 1024);
        jak_carbon_insert_prop_string(prop_ins, "hello", "object!");
        jak_carbon_insert_object_end(&as_prop_state);

    jak_carbon_insert_object_end(&state);

    jak_carbon_create_end(&context);

    as_json = jak_carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    jak_carbon_drop(&record);
    free(as_json);

    return 0;
}