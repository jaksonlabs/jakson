# Insertions into Objects

Key-value pairs (called properties) in object containers are added with an insertion context `carbon_insert`, defined in `carbon_insert.h`. An insertion context results either from a library function call, or is manually constructed.

**Library Functions**. Insertion contextes for object containers result from 

- another insertion context from an array that is requested to add a new object as a field by calling `carbon_insert_object_begin`, defined in `carbon_insert.h`, or 
- when an another insertion from an object context is requested to add a new object container as value for a property by calling `carbon_insert_prop_object_begin`, defined in `carbon_insert.h`, or 
- when any field is updated (and potentially re-written) as an object container by calling `carbon_update_set_object_begin`, defined in `carbon_update.h`.

**Manual Construction**. Another possibility is to construct insertion contextes manually, by calling `carbon_int_insert_create_for_object`, but this function is for internal usage and ist not typically called from client code.


> **Note**. Carbon does not support object containers as outer-most container (i.e., records are versioned array containers). Therefore, an object container must live in an array container (either a nested one, or the record itself), or is a value of a property of another object.

## Example

In the following example, a new record is created that returns an insertion context `ins`. This insertion context is used to add a single object. By adding this single object, a nested insertion context `nested_ins`, which is bound to that object, is used to add some properties to the single object. These properties are a string_buffer property, and an object property. The insertion of the latter results in another insertion context, `as_prop_state`, that is used to add a property to that object property. 

```c
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

    ins = carbon_create_begin(&context, &record, 
    	CARBON_KEY_NOKEY, CARBON_KEEP);

    nested_ins = carbon_insert_object_begin(&state, ins, 1024);
        carbon_insert_prop_string(nested_ins, "key", "value");
        prop_ins = carbon_insert_prop_object_begin(&as_prop_state, 
        	nested_ins, "as prop", 1024);
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
```

In the example from above, one object is added to the record via the insertion context `ins` that results from the record creation. The context `ins` is bound to the array container inside the record. To this array container, a single object is added by opening an (atomic) insertion block beginning with `carbon_insert_object_begin` and ending with `carbon_insert_object_end`. The begin requires a state object `state`, which is required to adjust the parent memory file position when calling `carbon_insert_object_end`. The begin takes a non-zero integer (here `1024`) as capacity for the object in number of bytes of the new-to-add object. Assuming this capacity would be `1`, the object container would look like `[{]0[}]`, when the insertion block would be immediatly ended, and could be optimized to `[{][}]` when calling a proper [record optimization](../../record-optimization.md). However, the result of a call to `carbon_insert_object_begin` returns an insertion context, `nested_ins` that is bound to an object container, which is used to add two properties (`key` and `as prop`). While the first is a string_buffer property with value `value`, the second is an object property, that must be created with another (atomic) insertion block like the parent object. The resulting insertion context, `prop_ins` that is bound to the object container for the value of the property with key `as prop`, is used to add another string_buffer property with key `hello`
 and value `object!`.

Afterwards the resulting record is printed to standard out, resulting in the following JSON string_buffer:

```json
{
   "key":"value",
   "as prop":{
      "hello":"object!"
   }
}
```

Finally, some cleanup operations are performed.

## Function Overview for Object-Container Insertion Contextes

Array and object containers are the most flexible container, capable of containing all field types including any other container. Since a Carbon record is in its core a versioned array container, object containers cannot be the record itself without beeinng an element of that array container. In simpler words, if a record contains of just a single object, then the corresponding object container is the first element in the record array container. 

> **Single Objects as Records**. From a callers perspective, a record with only one contained object in the record array container (i.e., the record looks like `[[][{]...[}][]]`) is treat by the API as it would be a single object container (i.e., the record behaves like `[{]...[}]`). This becomes obvious when evaluating dot-notated paths with [find](../../find-contents.md). Assume a record with a single object `{ "x":"y" }`, [find](../../find-contents.md) called for a path `x` will return `"y"`. Actually, the path `x` a convenient shortcut for `0.x` since the single object is stored internally as `[{ "x":"y" }]`.

The following functions of an insertion context `carbon_insert` are allowed to called (all defined in `carbon_insert.h`:

| Function                               | What is inserted
|----------------------------------------|-------------
| `carbon_insert_prop_null`               | a `null` constant
| `carbon_insert_prop_true`               | a `true` constant
| `carbon_insert_prop_false`              | a `false` constant
| `carbon_insert_prop_u8`                 | an `u8` integer
| `carbon_insert_prop_u16`                | an `u16` integer
| `carbon_insert_prop_u32`                | an `u32` integer
| `carbon_insert_prop_u64`                | an `u64` integer
| `carbon_insert_prop_i8`                 | an `i8` integer
| `carbon_insert_prop_i16`                | an `i16` integer
| `carbon_insert_prop_i32`                | an `i32` integer
| `carbon_insert_prop_i64`                | an `i64` integer
| `carbon_insert_prop_float`              | a `float32` number 
| `carbon_insert_prop_string`             | a string_buffer of `n` character
| `carbon_insert_prop_nchar`              | a prefix string_buffer of `n` character
| `carbon_insert_prop_binary`             | a user-defined binary string_buffer
| `carbon_insert_prop_object_{begin,end}` | an object container
| `carbon_insert_prop_array_{begin,end}`  | an array container
| `carbon_insert_prop_column_{begin,end}` | a column container

For convenience, the library provides two functions `carbon_insert_prop_unsigned` and `carbon_insert_prop_signed`, which internally call the best fitting `carbon_insert_prop_u...` resp. `carbon_insert_prop_i...` function for an input integer `x`. A function is best fitting in this context, if the functions encoding type requires the minimum amount of bytes to store `x`. For instance, `carbon_insert_prop_unsigned` called for `42` will internally call `carbon_insert_u8`, but `carbon_insert_prop_u16` if the input is `4242`. However, both `carbon_insert_prop_unsigned` and `carbon_insert_prop_signed` require some effort to figure out which particular function to call. Therefore, whenever possible, use the function from the table above rather than these two convenience functions.