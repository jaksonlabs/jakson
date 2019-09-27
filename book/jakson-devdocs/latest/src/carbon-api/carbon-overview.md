# Carbon Overview

Carbon is a binary data format built to encode full-compatible JSON-standard self-describing data in a space-efficient, traversable manner. 

## Example Encoding

To have a brief look at how Carbon stores JSON documents, consider the following example:

**JSON Document**
```json
{
   "title":"Back to the Future",
   "sub-title":null,
   "year":1985,
   "imdb-rating":8.5,
   "keywords":[
      "time travel",
      "delorean",
      "comedy"
   ],
   "release-dates":[
      1985, 1986, 1987,
      1992, 2008, 2010,
      2012, 2015, 2016
   ]
}
```

Using the Carbon encoding, the JSON document from above is binary-encoded as follows:

**Carbon Record**
```
[nokey]
[[]
	[{]
	   (5)[title] [s](18)[Back to the Future]
	   (9)[sub-title] [n]
	   (4)[year] [d][1985]
	   (11)[imdb-rating] [r][8.5]
	   (8)[keywords] [[] 
	         [s](11)[time travel] 
	         [s](8)[delorean]       
	         [s](6)[comedy]            
	      []]
	   (13)[release-dates] [2](9)(9)
	         [1985] [1986] [1987]
	         [1992] [2008] [2010]
	         [2012] [2015] [2016]
	[}]         
[]]
```

**Encoding Interpretation**

Here, a fixed-sized memory area is denoted with `[...]`, and variable-length memory areas are written as `(...)`. 

In short, the document from above is encoded as an intermediate record without any record identification (cf., `nokey`), that contains of exactly one element (cf., `[[] ... []]`). This element is one object (cf., `[{] ... [}]`) with six properties. 

The first property has a key of 5 characters (cf., `(5)`), which is variable-length encoded using 1B, named `title`. The value for that property is a string_buffer (cf. `[s]`) of the eighteen `(18)` character string_buffer `Back to the Future`. In addition to that, `sub-title` is a null field (cf. `[n]`), `year` is a fixed-length unsigned 2B integer property with value `1985`, and `imdb-rating` is a 4B floating point property. 

Further,  properties called `keywords` and `release-dates` are contained, which are both list types. The first is a list type for variable-length, potentially nesting data types (called *array*) while the second is a list type for fixed-length number types (called *column*). The array `keywords` consists of three string_buffer fields, and the `release-dates` column contains nine element slots (cf. second `[9]`) where all these slots are in use (cf. first `[9]`) over a fixed-number type of unsigned 2B integers (a `column-u16` type, cf. `[2]`). The contained nine values are then listed after the column header in one continuous memory area of nine 2B elements.

**Relationship to UBJSON**

Carbon uses a marker-based encoding that is heavily inspired by [UBJSON](http://www.ubjson.org) but uses variable-length encoding for any fixed-sized integer type, introduces trinary-logic for columns (aka fixed-type arrays), adds record identifcation and more. 

For ease of readability, the usage of indexes is not shown in the example from above. To have a more distinct comparison to [UBJSON](http://www.ubjson.org), read the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org).


## Brief Comparison to Alternatives

Carbon is similar to MongoDB's [BSON](www.bsonspec.org) w.r.t. its purpose, but is more similar to [UBJSON](www.ubjson.org) in its fundamental structure. However, in contrast to UBJSON, Carbon is designed for application in database systems rather than for data exchange between processes. As a consequence, Carbon includes a rich and well-defined set of update-in-place rules, and record identification semantics, and multi-version concurrency control. 

In direct comparison to both UBJON and [BSON](www.bsonspec.org), Carbon produces binary files with a smaller memory footprint [**CLAIM MUST BE SUPPORTED**] while at the same time allows traversals at least as fast as the fastest of both [**CLAIM MUST BE SUPPORTED**]. Like BSON, Carbon allows to query for data using an iterator-based access model, and dot-notated path expressions. 

In addition to [BSON](www.bsonspec.org), Carbon further allows to add a specialized index, the path index, to support record-level dot-notated path evaluations. 

Similar to [BSON](www.bsonspec.org) and UBJSON, Carbon produces a binary-encoded record that can be freely moved in memory without the need for pointer swizzling or pointer updates. 

### Columns vs. UBJSONs Optimized Arrays

A column container of Carbon is similar to [optimzed fixed-type & count arrays](http://ubjson.org/type-reference/container-types/#optimized-format-example-array) in [UBJSON](http://www.ubjson.org) but has some significant differences. 

A column is optimized for write-heavy operations, trinary logic, a small memory footprint, and allows abstract typing. In direct comparison to [UBJSON](http://www.ubjson.org), the following difference becomes clear:

- *Encoding for Write-Heavy Operations*. Optimization for write-heavy operations in columns is achieved by providing both, a capacity counter and an element counter. This enables sufficient memory managment for [overflow situtations](record-optimization/containers-and-memory.md), and [size optimizations](record-optimization.md) in general. 

- *Support of Trinary Logic*. A [UBJSON](http://www.ubjson.org) fixed-size array cannot intermix *null* values with number or boolean values. Since trinary logic is essential for database query predicates, column explictly allow intermixing *null* by encoding *null* as a distinct, reserved value of the element type domain. In [UBJSON](http://www.ubjson.org) special values, such as *null*, are always stored as their marker (e.g., `[n]` for *null*) and never double encoded as a value inside a list, as in Carbon. However, [UBJSON](http://www.ubjson.org) supports small memory footprint encoding of *null* value repetitions for *null*-value-only arrays of a particular length. This corner case is not supported by carbon containers, though.

- *Smaller Memory Footprint*. Finally, the encoding used for columns requires slightly less memory than fixed-size array of [UBJSON](http://www.ubjson.org), since column uses variable-length encoded counters, and does neighter require a dedicated optimization flag to mark an array as optimized, nor requires a dedicated marker to determine the elements type in arrays. The latter is achieved by providing effctively ten different built-in column types in Carbon rather than one generic that encodes the contained element type.

### Container Semantics 

Carbon supports two base storage types, *unsorted lists* (via arrays and columns), and *unsorted multi maps* (via objects). Using these two storage types, elements of more special abstract types (such as sorted sets, or sorted maps) can be stored in a Carbon record. Carbon ensures preserving of the semantic of a particular  special abstract type by *abstract typing*, which is an annotation several guarantees and properties to a container. Carbon records do not actually implement special abstract types, but allow to query whetever a particular guarantee is broken or not. 

Neither [UBJSON](http://www.ubjson.org) nor [BSON](www.bsonspec.org) support that kind semantic annotation. The closest to abstract typing, in the sense of providing an additional layer of semantic above some sequence of elements, is [REDIS](https://redis.io), which, however, supports only a subset of Carbons special abstract types: *unsorted maps*, *stacks* and *queues*, *unsorted sets*, and *multi sets*.

A more detailed view on the actual encoding of carbon columns can be found in the [Columnar Binary JSON (Carbon) specification](hhtp://www.carbonspec.org).

### User-Defined Data Type

Similar to both [UBJSON](http://www.ubjson.org) and [BSON](www.bsonspec.org), binary strings can be stored in Carbon records either as fields in arrays or values in objects. 

For instance, when storing a PNG-formatted image inside an array, [UBJSON](http://www.ubjson.org) and [BSON](www.bsonspec.org) store the raw binary string_buffer as-is without any further information. How, if, and where the actual data type is stored in a [BSON](www.bsonspec.org) file or [UBJSON](http://www.ubjson.org) file is delegated to the user. 

In contrast to both alternatives, Carbon has a well-defined process for that, which bundles type information directly to the user-defined binary string_buffer in an space-efficient way. In a nutshell, a binary string_buffer is stored as a tuple containing a type annotation and the raw binary string_buffer. The type annotation is either one constant integer that represents the Internet Media Type (*MIME* type), or a character string_buffer in case of an user-defined data type if the MIME type is unknown.

The benefit of allowing user-defined data types and storing the type information near the raw binary string_buffer is, that a system using Carbon is able to be extended towards support of particular user-defined types which are not natively supported by the system, such as image types, audio types, or mathematical types, such as matrices.

## More Details

If your are interested in more details on an implementation-independent level, or want to have more sharp details on the encoding differences of Carbon compared to alternatives, have a look at the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org).
