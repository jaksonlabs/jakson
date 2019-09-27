# From JSON

New Carbon records are created from a valid [JSON](https://tools.ietf.org/html/rfc8259) plain-text strings by calling `carbon_from_json` defined in `carbon.h`.


```c
// bin/examples-from-json

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    carbon record;
    err err;
    char *as_json;

    carbon_from_json(&record, "{\"msg\": \"Hello from JSON\"}", 
    	CARBON_KEY_NOKEY, NULL, &err);

    as_json = carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    carbon_drop(&record);
    free(as_json);

    return 0;
}
```

In the above example, `carbon_from_json` is called to construct a new record `record` that lives on the stack, given the [JSON](https://tools.ietf.org/html/rfc8259) string_buffer `"{"msg": "Hello from JSON"}"`. The record is for intermediate use, and therefore no primary key is used (`CARBON_KEY_NOKEY` and `NULL` as next parameter). In case of any problem (such as parsing issues), `carbon_from_json` will return `false` and will store a proper err state in `err`. Afterwards, the record is printed to standard out, and some cleanup is performed.

