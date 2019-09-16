# From JSON

New Carbon records are created from a valid [JSON](https://tools.ietf.org/html/rfc8259) plain-text strings by calling `jak_carbon_from_json` defined in `jak_carbon.h`.


```c
// bin/examples-from-json

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    jak_carbon record;
    jak_error error;
    char *as_json;

    jak_carbon_from_json(&record, "{\"msg\": \"Hello from JSON\"}", 
    	JAK_CARBON_KEY_NOKEY, NULL, &error);

    as_json = jak_carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    jak_carbon_drop(&record);
    free(as_json);

    return 0;
}
```

In the above example, `jak_carbon_from_json` is called to construct a new record `record` that lives on the stack, given the [JSON](https://tools.ietf.org/html/rfc8259) string `"{"msg": "Hello from JSON"}"`. The record is for intermediate use, and therefore no primary key is used (`JAK_CARBON_KEY_NOKEY` and `NULL` as next parameter). In case of any problem (such as parsing issues), `jak_carbon_from_json` will return `false` and will store a proper error state in `error`. Afterwards, the record is printed to standard out, and some cleanup is performed.

