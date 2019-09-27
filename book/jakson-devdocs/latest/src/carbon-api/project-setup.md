# Project Setup

The Carbon API is part of Jakson source code, but can be accessed in a library fashion. 

## Get the Sources

The starting point is to get the Jakson sources. Typically, this is done by cloning a stable version of Jaksons public git repository hosted at GitHub:

```bash
$ git clone https://github.com/jaksonlabs/jakson.git
```


### Modules versus Amalgamation

Depending on the configuration, the Carbon API can be fine-grained accessed via its modules (see *Structure* below), or accessed as single translation unit if *source amalgamation* is used (see *Source Amalgamation* below). 

### Jakson Project versus Third-Party

Since the Jakson core is build in a library fashon and modularized, Jakson (including Carbon) can be used by third-party projects. 

#### Inside Jakson
Inside the Jakson project, the Carbon API is accessible from any point where the following header is included:
```c
#include <jakson/jakson.h>
```

> For development, it is recommended to turn source amalgamation off because some IDEs will not recognize files in `src/` as part of the project otherwise. 

To turn off amalgamation, type `$ cmake  -DUSE_AMALGAMATION=OFF .`, and `$ cmake  -DUSE_AMALGAMATION=ON .` to enable amalgamation. In any case, the header `jakson/jakson.h` is sufficient to access all modules of Jakson.

#### Third-Party

For third-party projects, Carbon is used by one of two ways, adding the entire Jakson project as single-unit library, or by cherry-picking modules.

##### Jakson as Single-Unit Library

For third-party projects, it is recommended to configure Jakson as a single translation unit using source amalgamation, and to add the resulting two files, `jakson.h` and `jakson.c`, to the third-party project inclusion list. In this way, Jakson is used as a single-component library, and can be treated like this by third-party. To access the Carbon API, just include the header `jakson.h`.


##### Cherry-Picking Modules

> **Not Recommended**. Although it is possible for third-party projects to use particular Carbon modules by directly cherry-picking required modules, it is not recommended due to a non-trivial dependency graph between Carbon API modules and other modules outside the Carbon sub-system (e.g., err handling facilities). 

In a scenario where cherry-picking of modules is intended, including `jakson.h` is not sufficient if the desired effect is to exclude particular modules. Rather than just including `jakson.h` as a single translation unit, required module header files (including their transitive dependencies) must be included manually. For these cases it must be considered, that source amalgamation is explicitly not used and therefore runtime performance potential (due to exhaustive compiler optimization inside one huge tanslation unit) is left unexploited. 

Hence, cherry-picking modules must be done with the intention of decreasing the resulting binary size rather than (runtime) performance optimization.


## Structure

Public functionality that belongs to Carbon in Jakson is prefixed with `carbon`, such as `carbon.h` or `carbon_find.h`, and located in Jaksons source directoy `src/`. 

Like other components of Jakson, the Carbon implementation is modularized. 

These modules are

- `carbon`, the main entry point for Carbon records, which contains functionality to create, load, and store carbon records, open iterators, open atomic modification operations, accessing and modifying primary keys, and printing Carbon records
- `carbon_array_it`, provides iterator facilities over array containers in Carbon records, arrays, fields, and for values of properties inside objects, including iterator state information, element access, low-level data access, and modification operations
- `carbon_column_it`, provides iterator facilities over column containers in fields or values of properties inside objects, including iterator state information, bulk element access, low-level data access, and modification operations
- `carbon_object_it`, provides iterator facilities over object containers in Carbon records, arrays, fields, and for values of properties inside objects, including iterator state information, key and value type information and access, low-level data access, and modification operations
- `carbon_commit`, is about low-level functionality related to commit revision hashs for Carbon record, including functions to create, skip, read, and update these commit hash stored in memory files
- `carbon_dot`, is the module that implements dot-notated path objects, including manual creation, accessing, and parsing from user-defined character strings
- `carbon_field`, provides field type information, abstraction, stringification, property querying, and low-level memory files based functions for constants, containers, character strings, numbers, and binary data of user-defined types for fields, property values, and, where possible, for columns
- `carbon_find`, implements a exact query to a field, a property, or an object given a dot-notated path, and includes functions to query for the result type (if any), and safe data access 
- `carbon_insert`, is the module responsible for atomic data insertions to containers (arrays, columns, and objects), providing functions to insert fixed- and variable-length data, and nesting facilities
- `carbon_key`, provides functionalities to create, skip, read, write and update Carbon record primary keys on the low-level memory files abstraction layer
- `carbon_media`, is the module for media-type information of user-defined binary strings with functionalities to encode and decode pre-defined types (i.e., a subset of MIME typing) for embedded binary data
- `carbon_path`, is the concrete evaluator for *find* operations on dot-notated paths, internally evaluating a path via (array, column, or object) iterators, or by the use of a special (optional) path index above a Carbon record
- `carbon_path_index`, is the module realizing a path index, which focuses on query performance by statefull random access iteration and efficient string_buffer comparison to the cost of an increased memory footprint and loss of (binary) readability, in order to speedup *find* operations on a single Carbon record by indexing a records structure to exactly the form a path evaluation is done
- `carbon_printers`, is the conversion framework for Carbon record at a fine-grained level by exposing an "interface" to a printer comonent to enable printing of records in any user-defined way, including access to built-in printers for JSON with and without additional meta data, such as primary key information
- `carbon_revise`, is the module responsible for (abortable) atomic update operations (called *revisions*) on Carbon records by providing write-access to a Carbon records low-level memory file, primary key modifications, write-enabled iterator access, remove operations, and low-level Carbon record optimization functions to remove potentially existing memory reserves in a Carbon file
- `carbon_string`, is a low-level module for reading, writing, update, skip of strings in the low-level memory file of a Carbon record for several encoding types (e.g., with or without preceeding type marker)
- `carbon_update`, provides facilities to perform in-place update with potentially type-rewriting for fields, containers, and properties in an atomic manner

In addition to the modules from above, the module `carbon_int` contains internal structures and functions, such as iterator state manipulations or low-level field access, that are typically not called by user-code.

## Source Amalgamation

Source amalgamation is a technique that combines all source file of into one translation unit to improve runtime performance by enhance compiler optimizations. You find source amalgamation in database system SQLite, the JavaScript interpreter Duktape, and others. 

The downside of source amalgamation is more cumbersome debugging because a typical debugger is not optimized for such large source files. 

#### Inside Jakson

Whether amalgamation is turned on or off is not relevant for development purposes inside the Jakson project, since the projekt has an abstraction for its sources with or without enabled amalgamation. This abstraction works over inclusion of the header `jakson/jakson.h` which either includes all other module headers in case amalgamation is turned off, or is the amalgamation header itself.

With the exception of debugging and diagnostics, the effect of amalgamation in the Jakson project is mainly visible during performance tests, where amalgamation leads to a runtime performance increase of up to 40% compared to the same code where amalgamation is disabled.

#### Third-Party

For third-party projects, amalgamation is optional but the recommended process to include Carbon functionality. To create the source amalgamation of Jakson, run the following code after cloning the git respository and checking out a particular branch:

```bash
$ cmake  -DUSE_AMALGAMATION=ON .
```

With this command, source amalgamation is executed without running any compilation to it. The resulting single-translation module `jakson` is created, and stored in `include/jakson`. In the third-party project, `jakson.h` must be added to the (library) inclusion list, and `jakson.c` to the source list.

## Accessing Carbon Functionality

Independent of whether the Carbon API is accessed inside the Jakson project, or externally by third-party, inclusion of the single header `jakson/jakson.h` provides access to the entire functionality.

```c
// bin/examples-hello-carbon

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
    carbon_insert_string(ins, "Hello");
    carbon_insert_string(ins, "Carbon!");
    carbon_create_end(&context);

    as_json = carbon_to_json_compact_dup(&record);

    printf ("%s\n", as_json);

    carbon_drop(&record);
    free(as_json);

    return 0;
}

```

The example from above prints `["Hello", "Carbon!"]` to standard out.

## Code Listings

The following pages drive into particular use-cases and tutorials for the Carbon API. Each non-trivial example is supported by a source code example, which are additionally located in `examples/carbon-api/` in the Jakson repository.

To build these examples, type the following in the project root

```bash
$ cmake . && make examples
```

Afterwards, all examples executables are located in `bin` and prefixed with `example-`.