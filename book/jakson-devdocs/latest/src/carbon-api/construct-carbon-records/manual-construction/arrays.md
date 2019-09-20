# Insertions into Arrays

Fields in array containers are added with an insertion context `jak_carbon_insert`, defined in `jak_carbon_insert.h`. An insertion context results either from a library function call, or is manually constructed.

**Library Functions**. Insertion contextes for array containers result  

- from record creation by calling `jak_carbon_create_begin`, defined in `jak_carbon.h`, or
- when an another insertion context from another array is requested to add a new (nested) array as a field by calling `jak_carbon_insert_array_begin`, defined in `jak_carbon_insert.h`, or 
- when an another insertion from an object context is requested to add a new array container as value for a property by calling `jak_carbon_insert_prop_array_begin`, defined in `jak_carbon_insert.h` 
- when any field is updated (and potentially re-written) as an array container by calling `jak_carbon_update_set_array_begin`, defined in `jak_carbon_update.h`.

**Manual Construction**. Another possibility is to construct insertion contextes manually, by calling `jak_carbon_int_insert_create_for_array`, but this function is for internal usage and ist not typically called from client code.



## Example

In the following example, a new record (which is a wrapper for an array container) is created, and the insertion context `ins` as result of the call to `jak_carbon_create_begin` is used to add some elements to the record (array) container. One of these elements is a nested array container, which insertion over `ins` results in a new insertion context `nested_ins` that is bound to the nested array.

```c
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

    ins = jak_carbon_create_begin(&context, &record, 
    	JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

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
```

In the above example, at first and at last, two strings (`Hello` and `array!`) are added, between them a nested array is added inside an (atomic) array insertion block beginning with `jak_carbon_insert_array_begin` and ending with `jak_carbon_insert_array_end` to insert a nested array containing of two strings `you` and `nested`. The begin of this block requires a state object (`state`) that is used to adjust the parent memory file position when calling `jak_carbon_insert_array_end`, the parent insertion context (here `ins`) itself, and a non-zero integer (here `1024`) that defines the capacity in number of bytes of the new-to-add array. Assuming this capacity would be `1`, the array container would look like `[[]0[]]`, when the insertion block would be immediatly ended, and could be optimized to `[[][]]` when calling a proper [record optimization](../../record-optimization.md).


Afterwards the resulting record is printed to standard out, resulting in the following JSON string:

```json
[
   "Hello",
   [
      "you",
      "nested"
   ],
   "array!"
]
```

Finally, some cleanup operations are performed.

## Function Overview for Array-Container Insertion Contextes

Array and object containers are the most flexible container, capable of containing all field types including any other container. Since a Carbon record is in its core a versioned array container, elements can be added directly to a record as part of the record array container.

 The following functions of an insertion context `jak_carbon_insert` are allowed to called (all defined in `jak_carbon_insert.h`:

| Function                               | What is inserted
|----------------------------------------|-------------
| `jak_carbon_insert_null`               | a `null` constant
| `jak_carbon_insert_true`               | a `true` constant
| `jak_carbon_insert_false`              | a `false` constant
| `jak_carbon_insert_u8`                 | an `u8` integer
| `jak_carbon_insert_u16`                | an `u16` integer
| `jak_carbon_insert_u32`                | an `u32` integer
| `jak_carbon_insert_u64`                | an `u64` integer
| `jak_carbon_insert_i8`                 | an `i8` integer
| `jak_carbon_insert_i16`                | an `i16` integer
| `jak_carbon_insert_i32`                | an `i32` integer
| `jak_carbon_insert_i64`                | an `i64` integer
| `jak_carbon_insert_float`              | a `float32` number 
| `jak_carbon_insert_string`             | a string of `n` character
| `jak_carbon_insert_nchar`              | a prefix string of `n` character
| `jak_carbon_insert_binary`             | a user-defined binary string
| `jak_carbon_insert_object_{begin,end}` | an object container
| `jak_carbon_insert_array_{begin,end}`  | an array container
| `jak_carbon_insert_column_{begin,end}` | a column container

For convenience, the library provides two functions `jak_carbon_insert_unsigned` and `jak_carbon_insert_signed`, which internally call the best fitting `jak_carbon_insert_u...` resp. `jak_carbon_insert_i...` function for an input integer `x`. A function is best fitting in this context, if the functions encoding type requires the minimum amount of bytes to store `x`. For instance, `jak_carbon_insert_unsigned` called for `42` will internally call `jak_carbon_insert_u8`, but `jak_carbon_insert_u16` if the input is `4242`. However, both `jak_carbon_insert_unsigned` and `jak_carbon_insert_signed` require some effort to figure out which particular function to call. Therefore, whenever possible, use the function from the table above rather than these two convenience functions.

For more information on the internal encoding of arrays and their elements, see the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org).