# Carbon Records

Carbon records are implemented by the `carbon` structured defined in `src/carbon.h`. 

In a nutshell, Carbon records are a memory file (`memfile`), the memory block (`memblock`) to which this memory file writes and reads, an err and status structure (`err`), and some internal variables for book-keeping. A Carbon record is a flat object that is encoded according to the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org) by using functionality of the contained memory file to read and write safly in one continuous memory area managned by the memory block.  

Modification on Carbon records (including their creation) are atomic operations that may consists of several sub operations, such as insertion or a particular property. Each atomic modification is managed by a record-local revision context (`carbon_revise`), which is wrapped by a special context (`carbon_new`) for creation of new Carbon records.


## Abstract Types and Containers

Carbon supports two abstract types, *lists*  and *maps*, and three container types *arrays*, *columns*, and *objects*. Container types are abstract type implementations to store sequences of values, or value pairs. Using these container type as storage backend, more advanced abstract types can be annotated (see below).

Both, array containers and column containers, belong to the abstract *list* (more specifically, to an *unsorted list*) type, while an object container belongs to the abstract *map* (more specifically, to an *unsorted multi map*) type. 

A record in Carbon is a list implemented as array container plus some version information and identification. 

### Unsorted Lists

List types are used to store a a finite sequence of elements and may contain duplicates. List elements have a particular (user-defined) order on both the logical as well as the pyhsical level. A list is of variable size, which is determined by the type and size of its elements. Both list types are dynamically re-sizable at runtime. 

Carbon supports two list types as physical storage:

- **Arrays** support any Carbon type (even mixed inside the same array) and are optimized for generality. Each element is identified by a marker-based encoding by its own. Enumerating array elements is supported by forward (and backward) iterators but not for random access iterators, since the position (i.e., the offset) of a particular element is not known in advance.

- **Columns** are specialized array containers that are limited to fixed-length number types, and the boolean type (potentially intermixed with *null* values). In contrast to array containers and object containers, column containers have a fixed size, which is  computed by the number of contained elements and the element type that is shared across all contained elements. Hence, a per-element encoding is not required and random access iteration is supported, since the position (i.e., the offset) of a particular element is known in advance.

The concrete type *array* corresponde to [JSON arrays](https://tools.ietf.org/html/rfc8259), effectively encoding it in a reasonable binary form, though. Columns are a subset of the arrays w.r.t. to the capability of that they can contain. This restriction enables a more specialized encoding. 

### Unsorted Multi Maps

A map is a finite sequence of key-value pairs (called properties). Properties are unique in context of their parent object w.r.t. to their key name, i.e., no two properties with the same key shall exist in the same object. Properties inside objects have no particular order on the logical level, and a particular order at the pyhsical level. Like lists, maps have a variable size, which is determined by the type and size of its properties.

- **Objects** are key-value maps containing key-value pairs where the key is a (variable-length) string_buffer, and the value is of any Carbon type. 

The concrete type *object* corresponde to an [JSON object](https://tools.ietf.org/html/rfc8259), effectively encoding it in a reasonable binary form. 

### Specialized Abstract Types

Both list types are used as storage backend for more advanced abstract types, such as *sets* and *stacks*. Carbon supports the annotation of lists to mark them as a special abstract types. How this special abstract type are implemented concretely, is not defined by the [Columnar Binary JSON (Carbon) specification](hhtp://www.carbonspec.org), though. In simpler words, a list can be marked as set, and Carbon ensures that this semantic information not gets lost, but checking type-specific properties (such as uniqueness, or order) is not performed by Carbon. Whether a property is broken, can be queried, though. 

In addition to the built-in **unsorted lists** (i.e., array and column containers), and **unsorted multi maps** (i.e., object containers), the following specialized abstract types annotations are supported by Carbon:


- **Sorted Lists**. Sorted array/column containers, potentially with duplicates.

- **Unsorted Sets**. Duplicate-free array/column containers without particular order.

- **Sorted Sets**. Sorted, duplicate-free array/column.

- **Queues (FIFO)**. Unsorted array/column containers, potentially with duplicates.

- **Unsorted Maps**. Object containers without particular order and unique keys.

- **Sorted Maps**. Key-based alphanumeric sorted object with unique keys.

- **Sorted Multi Map**. Key-based alphanumeric sorted object with duplicate keys.

Please not that unsorted lists (array and column container) act like a stack (LIFO data structure) directly. A queue (FIFO data structure) is an unsorted list with application-dependent behavior.

## Constructions

The Carbon API provides three ways for construction of new Carbon records:

1. [Parsing a JSON plain-text into a new Carbon record](construct-carbon-records/from-json.md)
2. [Manual construction of a Carbon record by API functions](construct-carbon-records/manual-construction.md)

In addition, the third way is [reconstruction of a Carbon record by a byte stream](construct-carbon-records/from-byte-streams.md).

For the first two ways, a [record identification type](record-identification.md), and an [record optimization flag](record-optimization.md) must be specified. 

The [record identification type](record-identification.md) determines what kind of primary key is used for the new record. For ease of understanding, all records that are created in the following pages will have no primary key at all (controlled by the key type `CARBON_KEY_NOKEY`). What this means and how to use alternatives is covered in the [Section about Record Identification](record-identification.md). 

The [record optimization flag](record-optimization.md) controls what to do with reserved memory for nested vector-like structures `<data> <reserved>` and the tail buffer `<reserved>` beyond the carbon record `<carbon-record> <reserved>`. For ease of understanding, all records in the following pages will be left unoptimized (controlled by the flag `CARBON_KEEP`). More details on alternatives and when to use these, is described in the [Section about Record Optimization](record-optimization.md)


The next pages give a tutorial on each construction way. 

Let's start with parsing JSON files into Carbon records.