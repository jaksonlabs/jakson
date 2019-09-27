# Manual Construction

New Carbon records are created manually by calling `carbon_create_begin` and `carbon_create_end`, both defined in `carbon.h`.

## Empty Record Creation

```c
// bin/examples-manual-construction

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    carbon_new context;
    carbon record;
    char *as_json;

    carbon_create_begin(&context, &record, 
    	CARBON_KEY_NOKEY, CARBON_KEEP);
    carbon_create_end(&context);

    as_json = carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    carbon_drop(&record);
    free(as_json);

    return 0;
}
```

In the above example, an empty record `{}` is created by calling `carbon_create_begin` for a new record `record` to the construction context `context`, where the resulting record is for intermediate use without any primary key (`CARBON_KEY_NOKEY`) and no applied optimizations to reserved memory areas in the resulting record (`CARBON_KEEP`). Afterwards the record is printed to standard out, and some cleanup operations are performed.

As mentioned earlier, modifications (including constructions) are atomic operations. In case of constructions, any insertion of values must belong to the construction context `carbon_new` and be enclosed by `carbon_create_begin` and `carbon_create_end`. If a record must be modified *after* creation (e.g., by insertion of new fields), a new revision context (`revise`) must be created. In the typical case of construction of non-empty records, it is recommended to use the revision context that is part of the construction context `carbon_new` to avoid unnecessary call overhead. 

## Non-Empty Record Creation

Let's create a non-empty record. For this purpose, the return result of `carbon_create_begin` is used, which is an insertion context (`carbon_insert`) that belongs to the construction context `carbon_new`. Such an insertion context is used to add elements to containers, i.e., elements to arrays or columns, and properties to objects.

```c
// bin/examples-manual-construction-non-empty

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    carbon_new context;
    carbon record;
    carbon_insert *ins;
    char *as_json;

    ins = carbon_create_begin(&context, &record, 
    	CARBON_KEY_NOKEY, CARBON_KEEP);
    
    carbon_insert_string(ins, "Hello, Carbon!");
    carbon_insert_unsigned(ins, 42);
    carbon_insert_null(ins);
    
    carbon_create_end(&context);

    as_json = carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    carbon_drop(&record);
    free(as_json);

    return 0;
}
```

To have a full compatibility to [JSON RFC 8259](https://tools.ietf.org/html/rfc8259) and to avoid array encoding as objects, like [BSON](www.bsonspec.org) does, a Carbon records most outer container is an array, and `carbon_create_begin` returns an insertion context `ins` for exactly that array. The benefit of this procedure is a very consistent workflow with insertion contextes and code structure in general. The drawback is that for single-object-only records, internally two bytes are wasted (namely the markers `[` and `]` that make up a record).

Consequentially, the example from above will result in the JSON file `["Hello, Carbon!", 42, null]` that is printed to standard out.

> **Empty records are empty JSON objects**. In the first example, an empty record was created and the resulting JSON string_buffer was `{}` rather than `[]`. Although the latter is correct in terms of internal encoding, the first is the correct resulting string_buffer for empty records. This is due to a design decision that reflect the common sense of an empty JSON as an empty document rather than an empty array. In cases where this behavior does not match the applications requirements, a modified [JSON printer](../conversion-to-json.md) is a good solution.

## Insertion Contextes

An insertion context `carbon_insert`, defined in `carbon_insert.h`, is the central context for insertion of values of any kind. Which value type can be inserted depends on the insertion context container context, though. 

### Revision Contextes

Each container (i.e., arrays, columns, and objects) allows getting a new insertion context for modifications *if* the caller has *write* access to the Carbon record at hand. For existing records, a *write* access must be aquired first. Such record modifications require a revision context (`carbon_revise`, defined in `carbon_revise.h`), and are described in the [Section about Carbon record modification](../modify-contents.md). For construction operations, *write* access is always the case, since any construction context `carbon_new` has its own revision context `carbon_revise` built-in. 

If only *read* access is availble to a record, any call to the insertion context will fail.

### Container-Dependency

Depending on the container in which the insertion context was create, that insertion context allows a particular set of insertion functions to be called. 

As rule of thumb, all insertion operations that do *not* insert a property (i.e., a key-value pair that is exclusive for object containers) are available for array containers. The reason is that array containers are the most flexible container, capable of containing all field types including any other container. 

The same holds for values of properties inside object containers, but an insertion context that belongs to an object cannot insert pure-fields without any key. 

The most efficient, yet most restricted, container, columns, are only capable of insertion operations of fields that are fixed-length numbers (integers, and floats), boolean values, and nulls. Any other insertion call to columns (especially insertion of (character/binary) strings, arrays, columns, and objects) will be rejected.

The [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org) provides a good overview about type support of indvidual containers.

In the next pages, insertion contextes for each container type of a Carbon record is covered.





