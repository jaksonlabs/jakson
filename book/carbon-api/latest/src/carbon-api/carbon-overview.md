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

The first property has a key of 5 characters (cf., `(5)`), which is variable-length encoded using 1B, named `title`. The value for that property is a string (cf. `[s]`) of the eighteen `(18)` character string `Back to the Future`. In addition to that, `sub-title` is a null field (cf. `[n]`), `year` is a fixed-length unsigned 2B integer property with value `1985`, and `imdb-rating` is a 4B floating point property. 

Further,  properties called `keywords` and `release-dates` are contained, which are both list types. The first is a list type for variable-length, potentially nesting data types (called *array*) while the second is a list type for fixed-length number types (called *column*). The array `keywords` consists of three string fields, and the `release-dates` column contains nine element slots (cf. second `[9]`) where all these slots are in use (cf. first `[9]`) over a fixed-number type of unsigned 2B integers (a `column-u16` type, cf. `[2]`). The contained nine values are then listed after the column header in one continuous memory area of nine 2B elements.

**Relationship to UBJSON**

Carbon uses a marker-based encoding that is heavily inspired by [UBJSON](http://www.ubjson.org) but uses variable-length encoding for any fixed-sized integer type, introduces trinary-logic for columns (aka fixed-type arrays), adds record identifcation and more. 

For ease of readability, the usage of indexes is not shown in the example from above. To have a more distinct comparison to [UBJSON](http://www.ubjson.org), read the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org).


## Brief Comparison to Alternatives

Carbon is similar to MongoDB's [BSON](www.bsonspec.org) w.r.t. its purpose, but is more similar to [UBJSON](www.ubjson.org) in its fundamental structure. However, in contrast to UBJSON, Carbon is designed for application in database systems rather than for data exchange between processes. As a consequence, Carbon includes a rich and well-defined set of update-in-place rules, and record identification semantics, and multi-version concurrency control. 

In direct comparison to both UBJON and BSON, Carbon produces binary files with a smaller memory footprint [**CLAIM MUST BE SUPPORTED**] while at the same time allows traversals at least as fast as the fastest of both [**CLAIM MUST BE SUPPORTED**]. Like BSON, Carbon allows to query for data using an iterator-based access model, and dot-notated path expressions. 

In addition to BSON, Carbon further allows to add a specialized index, the path index, to support record-level dot-notated path evaluations. 

Similar to BSON and UBJSON, Carbon produces a binary-encoded record that can be freely moved in memory without the need for pointer swizzling or pointer updates. 

## More Details

If your are interested in more details on an implementation-independent level, or want to have more sharp details on the encoding differences of Carbon compared to alternatives, have a look at the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org).