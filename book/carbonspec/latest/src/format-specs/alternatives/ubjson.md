# Universal Binary JSON

## UBJSON Running Example


(size-optimized UBJSON)

```
[object-begin, 1B]
   [small-int, 1B] [str-len, 1B][title, 5B] [string-field, 1B]
                        [small-int, 1B] [str-len, 1B][Back to the Future, 18B]
   [small-int, 1B] [str-len, 1B][sub-title, 9B] [null-field, 1B]
   [small-int, 1B] [str-len, 1B][year, 4B] [short-integer-field, 1B][1985, 2B]
   [small-int, 1B] [str-len, 1B][imdb-rating, 11B] [float-field, 1B][8.5, 4B]
   [small-int, 1B] [str-len, 1B][keywords, 8B] 
      [array-begin, 1B] [strong-type-marker, 1B] [string-type, 1B]
         [small-int, 1B] [str-len, 1B][time travel, 11B] 
         [small-int, 1B] [str-len, 1B][delorean, 8B]       
         [small-int, 1B] [str-len, 1B][comedy, 6B]            
      [array-end, 1B]
   [small-int, 1B] [str-len, 1B][release-dates,13B] 
      [array-begin, 1B] [count-marker, 1B] [num-elems, 1B] [strong-type-marker, 1B]
         [1985, 2B] 
         [1986, 2B] 
         [1987, 2B]
         [1992, 2B] 
         [2008, 2B] 
         [2010, 2B]
         [2012, 2B]
         [2015, 2B]
         [2016, 2B]
[object-end, 1B]  
```

155 byte

## Comparison to Carbon

### Design Goals

#### Commonalities

#### Differences

- UBJSON is not intented for writes, while Carbon emphasises in-place modifications including (optional) reserved memory ranges as capacity buffers to avoid reallocation during updates
- No specification on update semantics for UBJSON, while there is a detailed specification on atomic operations, update behavior and version control for Carbon

### Encoding

#### Commonalities

- Carbon and UBJSON are very similar in its core structure (similar containers in general), similar use of markers. 
- Support of constrainted arrays ("strongly typed arrays"), but with different optimization goals in mind

#### Differences

In direct comparison, Carbon focus more on semantics (e.g., embedding additional meta-data) and focus more on support for operations of typical operational document stores (e.g., on-the-fly updates), while UBJSON focusses on efficient data exchange between two actors. Both, Carbon and UBJSON share the concept of a minimalistic type-support to have full compatibility to the latest JSON specification. However, Carbon adds one additional data type (`custom`) `binary` to enable user-defined function (UDF) support on user-defined types, though.

- Carbon supports three-valued logic for `boolean` values, and encodes `null` values not only for the JSON constant *null* but also as valid value in strongly typed arrays (e.g., intermixing of `null` and number values in `column` containers of Carbon). UBJSON cannot store a `null` value in a strongly typed array of a particular non-null type, and must fallback to size-inefficient standard arrays in UBJSON. 
- Variable-length integer encoding for string lengths, or counters in Carbon
- Encoding of binary data: while in UBJON a (strongly typed) unsigned 8-bit integer array is used, in Carbon there is a dedicated type `binary` (resp. `custom binary`) that encoded the actual binary string data type (i.e., the *what is stored* information gets not lost in Carbon as it does in UBJSON)
- Revision numbering as dedicated (variable-length) meta-data field in Carbon
- Record identification via (optional) primary key as meta-data field in Carbon, and defined semantic about record identity
- Overall, typically smaller binary size for Carbon files compared to UBJSON files
- For Carbon, the evaluation of dot-path expressions is well-defined and specified
- Carbon specificies two way to express absence of data, `null` and `undef` where the first one maps to user-defined data and the second is a dedicated result value for dot-path expressions.
