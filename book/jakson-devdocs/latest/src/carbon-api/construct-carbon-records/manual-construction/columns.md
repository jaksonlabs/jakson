# Insertions into Columns

A column container is a specialized container that is only capable of containing fixed-size number types and boolean values (both potentially intermixed with *null* values). Neither nested arrays, nor column containers, nor  objects, nor strings can be elements of a column. The reason is that none of these types has a size that is known just by their type. A fixed size is required for column element types since a column is specialized for random access to its elements. Whenever possible, column containers should be prefered over array containers.

Elements in column containers are added with an insertion context `carbon_insert`, defined in `carbon_insert.h`. An insertion context results either from a library function call, or is manually constructed.

**Library Functions**. Insertion contextes for column containers result from

- another insertion context from an array is requested to add a new (nested) column as a field by calling `carbon_insert_column_begin`, defined in `carbon_insert.h`, or 
- when an another insertion from an object context is requested to add a new column container as value for a property by calling `carbon_insert_prop_column_begin`, defined in `carbon_insert.h` 
- when any field is updated (and potentially re-written) as an column container by calling `carbon_update_set_column_begin`, defined in `carbon_update.h`.

**Manual Construction**. Another possibility is to construct insertion contextes manually, by calling `carbon_int_insert_create_for_column`, but this function is for internal usage and ist not typically called from client code.

> **Note**. Carbon does not support column containers as outer-most container (i.e., records are versioned array containers). Therefore, a column container must live in an array container (either a nested one, or the record itself), or is a value of a property of another object.

## Example

In the following example, a new record (which is a wrapper for an array container) is created, and the insertion context `ins` as result of the call to `carbon_create_begin` is used to add a new column container for unsigned 32bit integers. One of these elements is a *null* value. 

```c
// bin/examples-manual-construction-columns

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
    carbon_new context;
    carbon record;
    carbon_insert *ins, *nested_ins;
    carbon_insert_column_state state;
    char *as_json;

    ins = carbon_create_begin(&context, &record, 
    	CARBON_KEY_NOKEY, CARBON_KEEP);

    nested_ins = carbon_insert_column_begin(&state, ins, 
    	CARBON_COLUMN_TYPE_U32, 1024);

        carbon_insert_u32(nested_ins, 23);
        carbon_insert_null(nested_ins);
        carbon_insert_u32(nested_ins, 42);
    
    carbon_insert_column_end(&state);

    carbon_create_end(&context);

    as_json = carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    carbon_drop(&record);
    free(as_json);

    return 0;
}
```

In the above example, a column container is created as single element to the record array container. This creation is an (atomic) column insertion block beginning with `carbon_insert_column_begin` and ending with `carbon_insert_column_end` to insert the column in `ins`. A call to the begin requires a state object (`state`) that is used to adjust the parent memory file position when calling `carbon_insert_column_end`, a *column type* (here `CARBON_COLUMN_TYPE_U32`, see below), and a non-zero integer (here `1024`) that defines the capacity in number of bytes of the new-to-add column. Potentially reserved but unused memory for that capacity is optimized when calling a proper [record optimization function](../../record-optimization.md). The result of that block beginning function returns an insertion context (`nested_ins`) that is bound to the new-to-add column container. This insertion context is used to add two unsigned integers (`23` and `42`), and between them a *null* value. 

Afterwards the resulting record is printed to standard out, resulting in the following JSON string_buffer:

```json
[
   23,
   null,
   42
]
```

Finally, some cleanup operations are performed.

## Specialized Encoding

As mentioned before, a column container is a restricted yet more specialized and faster list container.

### Null Values

The *null* value is a dedicated type-dependent value from the type domain that is reserved to encode absence of data. 

## Function Overview for Array-Container Insertion Contextes

Array and object containers are the most flexible container, capable of containing all field types including any other container. Since a Carbon record is in its core a versioned array container, elements can be added directly to a record as part of the record array container.

 The following functions of an insertion context `carbon_insert` are allowed to called (all defined in `carbon_insert.h`:

| Function                               | What is inserted
|----------------------------------------|-------------
| `carbon_insert_null`               | a `null` constant
| `carbon_insert_true`               | a `true` constant
| `carbon_insert_false`              | a `false` constant
| `carbon_insert_u8`                 | an `u8` integer
| `carbon_insert_u16`                | an `u16` integer
| `carbon_insert_u32`                | an `u32` integer
| `carbon_insert_u64`                | an `u64` integer
| `carbon_insert_i8`                 | an `i8` integer
| `carbon_insert_i16`                | an `i16` integer
| `carbon_insert_i32`                | an `i32` integer
| `carbon_insert_i64`                | an `i64` integer
| `carbon_insert_float`              | a `float32` number 
| `carbon_insert_string`             | a string_buffer of `n` character
| `carbon_insert_nchar`              | a prefix string_buffer of `n` character
| `carbon_insert_binary`             | a user-defined binary string_buffer
| `carbon_insert_object_{begin,end}` | an object container
| `carbon_insert_array_{begin,end}`  | an array container
| `carbon_insert_column_{begin,end}` | a column container

For convenience, the library provides two functions `carbon_insert_unsigned` and `carbon_insert_signed`, which internally call the best fitting `carbon_insert_u...` resp. `carbon_insert_i...` function for an input integer `x`. A function is best fitting in this context, if the functions encoding type requires the minimum amount of bytes to store `x`. For instance, `carbon_insert_unsigned` called for `42` will internally call `carbon_insert_u8`, but `carbon_insert_u16` if the input is `4242`. However, both `carbon_insert_unsigned` and `carbon_insert_signed` require some effort to figure out which particular function to call. Therefore, whenever possible, use the function from the table above rather than these two convenience functions.

For more information on the internal encoding of arrays and their elements, see the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org).
