# Insertions into Arrays

Fields in array containers are added with an insertion context `jak_carbon_insert`, defined in `jak_carbon_insert.h`. An insertion context results either from a library function call, or is manually constructed.

**Library Functions**. Insertion contextes for array containers result from record creation by 

- calling `jak_carbon_create_begin`, defined in `jak_carbon.h`
- when an insertion context is requested to add a new (nested) array container as a field inside another array by calling `jak_carbon_insert_array_begin`, defined in `jak_carbon_insert.h` 
- when any field is updated (and potentially re-written) as an array container by calling `jak_carbon_update_set_array_begin`, defined in `jak_carbon_update.h`.

**Manual Construction**. Another possibility is to construct insertion contextes manually, by calling `jak_carbon_int_insert_create_for_...`, but these function are for internal usage and are not typically called from client code.

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

In the above example, at first and at last, two strings (`Hello` and `array!`) are added, between them a nested array is added inside an (atomic) array insertion block beginning with `jak_carbon_insert_array_begin` and ending with `jak_carbon_insert_array_end` to insert a nested array containing of two strings `you` and `nested`. The begin of this block requires a state object (`state`) that is used to adjust the parent memory file position when calling `jak_carbon_insert_array_end`, the parent insertion context (here `ins`) itself, and a non-zero integer (here `1024`) that defines the capacity in number of bytes of the new-to-add array. 

In case not enough capacity is given for the final nested array contents, the remaining memory area after the new-to-add array is moved-in-place towards the end of the underlying memory file. This movement may result in a reallocation of the entire underlying memory block, if the tailing buffer of the Carbon record is exhausted. Otherwise, in case of that more capacity is given than actually used for the new-to-add array, reserved (zero-valued) memory remains as tail between the end of the new-to-add array and the next element in the parent array (i.e., the string field `array!`). 

Both reserved memory areas, the potentially reserved memory between the new-to-add array and the string field `array!`, and the tailing buffer of the Carbon record can be removed by specifying other [optimization flags](carbon-api/record-optimization.md) than `JAK_CARBON_KEEP`, or by manually calling `jak_carbon_revise_shrink` resp. `jak_carbon_revise_pack`  (both defined in `jak_carbon_revise.h`).


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

Array containers are the most flexible container, capable of containing all field types including any other container. The following functions of an insertion context `jak_carbon_insert` are allowed to called (all defined in `jak_carbon_insert.h`:

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

For convenience, the library provides two functions `jak_carbon_insert_unsigned` and `jak_carbon_insert_signed`, which internally call the best fitting `jak_carbon_insert_u...` resp. `jak_carbon_insert_i...` function for an input integer `x`. A function is best fitting in this context, if the functions encoding type requires the minimum amount of bytes to store `x`. For instance, `jak_carbon_insert_unsigned` called for `42` will internally call `jak_carbon_insert_u8`, but `jak_carbon_insert_u16` if the input is `4242`.

For more information on the internal encoding of arrays and their elements, see the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org).