# Containers and Memory Management

In its core, a Carbon record is a set of nested vector-like structures `<data> <reserved>` where one memory area slice contains some data (`<data>`) which is typically followed by a (potentially empty) sequence of zero-valued bytes making up capacities (`<reserved>`). These capacities are used to hold some (unused) bytes in reserve in order to avoid to-often reallocation when constructing or modifying records. Especially, the entire Carbon record itself follows this approach (i.e., a Carbon file looks like `<carbon-record> <reserved>`), where the reserved memory beyond the `<carbon-record>` is called the *tail buffer*. The tail buffer is used as every other reserved piece of memory, namely to buffer growth of data, but in addition is also used as temporary (already allocated) buffer in the heap. If possible, the tail buffer is used as intermediate space for re-writing purposes. A client typically does neither notice these reserved memories (with exception of the binary size), nor access these buffers directly. However, this reserved memory is target of several optimization options, as later shown in the [Section about Record Optimization](record-optimization.md).

## Memory Management

A container in memory is a range of bytes starting with some marker (say `[{]`) and ending at some point, either with an end marker (say `[}]`) or without an end marker if the container is fixed sized (i.e., the container is a column). Any container requests for a *capacity* during contstruction, that determines the `<reserved>` number of bytes for that container. 

### Capacity Management in Containers

For memory areas that form a single container where `[{]` and `[}]` do not follow directly after each other (i.e., the container is empty and has no reserved memory), there the container looks like the following in memory:

```
     
     [{] xx...x 00...0 [}]
         |      |
         |      └ <reserved> 
         └ <data>                

```

The capacity `<reserved>` shrinks from left to right if the `<data>` area grows towards `[}]`. 

**Overflow**: In case `<reserved>` is exhausted for growing `<data>`, additional space is required to store `<data>` without overwritting `[}]`. The number of bytes that cannot be written directly without overwritting `[}]` is called the overflow number *k*. Even if *k* is large, this does *not* necessarily require a reallocation of the underlying memory block, because of the records tail buffer. To understand, when an overflow triggers an reallocation the capactiy management for records must be understood.

### Capacity Management for Records

To understand when a reallocation of the memory block is required, and when just a memory move is performed when modifying containers of the record, the record layout in the memory block must be considered. For ease of understanding say the record starts with `[<]` and ends with `[>]`. 

For abstraction, assume that all containers (including markers, `<data>`, and  `<reserved>`) in a record are represented by continuous memory range `<containers>`, and that the memory area managed by the memory block starts at `[B]` and ends at `C`. Then the record looks like the following in memory:

```
     
     [B] [<] xx...x [>] 00...0 [E]
             |          └ <buffer>
             |
             └ <data>                

```

In case of an overflow of size *k* inside one container in `<containers>`, `<containers>` is enlarged by *k* bytes towords `[E]` by splitting `<containers>` at the overflow point in a `<head>` and `<tail>` part, and by moving `<tail>` *k* byte towards `[E]`, effectively shrinking the records tail buffer `<buffer>` left to right. This procedure is possible and solves the overflow situtation *without* reallocation of the underyling memory block, as long as the tail buffer `<buffer>` in the memory block exists. If `<buffer>` is exhausted, the memory block must be reallocated, which leads to a new tail buffer of a particular size. 

> *Note*: A Carbon record structure `carbon` can live on both the stack and the heap, but the underlying memory block, which is used to encode the actual record, always lives on the heap. Even in cases of reallocation, the API client must not care about pointer management.


### Capacity Optimization

Both the `<reserved>` memory area in each container, and the tail buffer `<buffer>` may be removed by specifying a [optimization post-processing step](carbon-api/record-optimization.md), which can be triggered by defining optimization flags unequal to `CARBON_KEEP`, or by manually calling `carbon_revise_shrink` resp. `carbon_revise_pack`  (both defined in `carbon_revise.h`).
