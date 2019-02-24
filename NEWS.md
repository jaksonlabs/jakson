## 0.1.00.07 [2019-XX-XX]
- Add archive visitor framework, see [archive visitor framework](include/carbon/carbon-archive-visitor.h).
- Add in-memory key-value pair representation of unstructured JSON-like objects 
  (which still use [object identifiers](include/carbon/carbon-oid.h) and encoded strings) in preparation to 
  print CARBON files in typical key-value pair structure, see [encoded documents](include/carbon/carbon-encoded-doc.h).
    - Used [archive visitor framework](include/carbon/carbon-archive-visitor.h) to convert 
      [CARBON archives](include/carbon/carbon-archive.h) to an
      [encoded document collection](include/carbon/carbon-encoded-doc.h)

## 0.1.00.06 [2019-02-23]
- Completed compressor framework including embedding into carbin archive operations 
  (e.g., automatically call <code>encode</code> or <code>decode</code> when needed)
    - Compressor _none_ works fully, _huffman_ still cannot <code>decode</code>
- Implement query function to archives
    - *String Identifier Scan*: Iteration through all string ids (including offset and string length) 
      stored in an archives string table, see <code>carbon_query_scan_strids</code>. Iterator model 
      is vector-based to minimize I/O calls to the underyling disk file, 
      see [carbon-strid-iter_t](include/carbon/carbon-strid-iter.h)
    - *String Fetch*: Fetch a stored (and potentially compressed) string in an archives string 
      table given its string identifier. For generic and safe calls, this fetch performs *full scan*
      iteration using [carbon-strid-iter_t](include/carbon/carbon-strid-iter.h) seeking for the desired
      string id (if a string id is found), see <code>carbon_query_fetch_string_by_id</code>. 
      For fast and unsafe calls having the offset to the desired string id in the archives string table 
      at hand, a _random access_ into the archives underlying disk file is performed, see
      <code>carbon_query_fetch_strings_by_offset</code>. To avoid too many open files and to guarantee 
      thread-safe behavior, such an unsafe operation is invoked via a 
      [carbon-io-context_t](include/carbon/carbon-io-context.h). In both case, the string
      gets decompressed using the compressor framework when it gets fetch into main memory.
  - *Id Fetch*: Fetch string ids where the referenced string matches a particular condition,
      see <code>carbon_query_find_ids</code>. Such a condition, implemented as (vectorized) predicate 
      function, can be freely defined by extending the [String Predicate Framework](include/carbon/carbon-string-pred.h) with
      user-defined predicates. There are already some [built-in predicates](include/carbon/string-pred),
      such as [equals](include/carbon/string-pred/carbon-string-pred-equals.h) or 
      [contains](include/carbon/string-pred/carbon-string-pred-contains.h). Internally, a string
      identifier scan is invoked. For the entire vector for such an iteration step, a string
      fetch with random access is invoked, yielding the corresponding uncompressed strings to the
      string identifiers inside the vector. For all of these strings, the predicate is invoked
      resulting in the indices inside the vector where the strings match the condition. Marked
      indices are added to the result set. Depending on the predicate and the users choice, a limit
      to the result set size can be set, stopping the fetch potentially earlier.
- Implemented iterator model to iterate trough CARBON archives in order to fully exploit zero-cost access
  to columnar stored key and value data, see [iterators](include/carbon/carbon-archive-iter.h). 

## 0.1.00.05 [2019-02-13]
- **Compressor Framework** refactored compressor framework. 
    - Extension with new compressors in [carbon-compressor.c](src/carbon/carbon-compressor.c)
    - Current compressors are located in [include/carbon/compressor](include/carbon/compressor)
    - Available compressors now listed via `carbon-tool list compressors`,
      and selection is done via `carbon-tool convert --use-compressor=$NAME` where 
      `$NAME` is from the list above 

## 0.1.00.04 [2019-02-13]
- Unique object identifiers added in CARBON files, see [oid.c](src/carbon/carbon-oid.c)
- Fine-grained switched to turn on and turn of logging, see [INSTALL.md](INSTALL.md)
- Default build type is now release
- Contains the following bug fixes 
    - [size-optimized (Huffman) fails in edge cases](https://github.com/protolabs/libcarbon/issues/1)

## 0.1.00.00 [2019-02-07]

- Initial standalone release
- Set license to MIT
- Refactoring and cleanups
