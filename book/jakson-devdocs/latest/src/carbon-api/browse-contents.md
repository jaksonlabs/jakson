# Browse Contents

To have a full compatibility with the [JSON RFC 8259](https://tools.ietf.org/html/rfc8259) and to allow  consistent data access with less special cases, a Carbon record is always a (versioned) multiset according to the [Columnar Binary JSON Specification](http://www.carbonspec.org) . This multiset may contain arbitary constant values (such as unsigned 32bit integers, or nulls) and other (potentially nested) set or map data structures in a user-defined order. 

## Data Access

This library gives access to nested data in two ways:

- **Iteration**. Data stored in *set* data structures is accessed by `array_it` bi-directional iterators and `column_it` random-acess iterators, and data stored in *map* data structures is acces by `object_it` bi-directional iterators. *Values* stored in these structures are accessed with function provided by an container-specific iterator. *Keys* are character strings that can be accessed alongside their *values* with `object_it` in a similar fashion
- **Lookup**. In case the path to a specific value or container (map/set) is known, this library supports user-defined dot-notated paths to access values, or iterators to containers (`array_it`, `column_it`, and `object_it`). Paths are organized by `carbon_dot_path` structures and optionally parsed from a user-defined character string.

## Mutable Access

