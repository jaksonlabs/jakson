// bin/examples-manual-construction-objects

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    carbon_new context;
    carbon record;
    carbon_insert *ins, *nested_ins, *prop_ins;
    carbon_insert_object_state state, as_prop_state;
    char *as_json;

    ins = carbon_create_begin(&context, &record, CARBON_KEY_NOKEY, CARBON_KEEP);

    nested_ins = carbon_insert_object_begin(&state, ins, 1024);
        carbon_insert_prop_string(nested_ins, "key", "value");
        prop_ins = carbon_insert_prop_object_begin(&as_prop_state, nested_ins, "as prop", 1024);
        carbon_insert_prop_string(prop_ins, "hello", "object!");
        carbon_insert_object_end(&as_prop_state);

    carbon_insert_object_end(&state);

    carbon_create_end(&context);

    as_json = carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    carbon_drop(&record);
    free(as_json);

    return 0;
}