# Derived Abstract Types

Annotation of container to mark a abstract type is done by replacing the containers begin marker with a special marker (called *derivation marker*) that declares the container as a particular abstract type. 

For instance replacing the object container begin marker `[{]` by the derivation marker `[~]` declares the object container as `sorted-multimap`. 

A derived abstract type is an abstract type that is neither an unsorted multi set (`unsorted-multiset`) nor an unsorted multi map (`unsorted-multimap`), and that marks the semantics for distinct values (or key), or a sorted sequence of elements, or both. 

> **Note**: Carbon itself does *not* provide any way to make these semantics work, i.e., the caller must manage the logic to provide distinct values or sorting. The responsibility of the Carbon format is to annotate a container *to have* these semantics. However, a library for Carbon *may* provide functionality to check wheter a particular guarantee holds.

Carbon supports the following derived abstract types from the abstract base type `unsorted-multiset` stored in `array`/`column` containers:

- sorted multisets (`sorted-multiset`)
- unsorted sets (`unsorted-set`)
- sorted sets (`sorted-set`)

Carbon supports the following derived abstract types from the abstract base type `unsorted-multimap` stored in `object` containers:

- sorted maps (`sorted-map`)
- sorted multi maps (`sorted-multimap`)
- unsorted map (`unsorted-map`)
- sorted map (`sorted-map`)



