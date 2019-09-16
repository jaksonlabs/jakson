// bin/examples-from-json

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    jak_carbon record;
    jak_error error;
    char *as_json;

    jak_carbon_from_json(&record, "{\"msg\": \"Hello from JSON\"}", JAK_CARBON_KEY_NOKEY, NULL, &error);

    as_json = jak_carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    jak_carbon_drop(&record);
    free(as_json);

    return 0;
}