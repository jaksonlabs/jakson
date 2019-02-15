## 0.1.00.06 [2019-02-XX]
- Completed compressor framework including embedding into carbin archive operations 
  (e.g., automatically call <code>encode</code> or <code>decode</code> when needed)
    - Compressor _none_ works fully, _huffman_ still cannot <code>decode</code>
- Implement query function to archives
    - *String Iteration*: Iteration through all string ids (including offset and string length) 
      stored in an archives string table. Iterator model is vector-based to minimize I/O calls to 
      the underyling disk file, see [carbon-strid-iter_t](include/carbon/carbon-strid-iter.h)
    - *String Fetch*: Fetch a stored (and potentially compressed) string in an archives string 
      table given its string identifier. For generic and safe calls, this fetch performs *full scan*
      iteration using [carbon-strid-iter_t](include/carbon/carbon-strid-iter.h) seeking for the desired
      string id (if a string id is found). For fast and unsafe calls having the offset to the
      desired string id in the archives string table at hand, a random access into the archives
      underlying disk file is performed. To avoid too many open files and guarantee thread-safe
      behavior, such an unsafe operation is invoked via a 
      [carbon-io-context_t](include/carbon/carbon-io-context.h). In both case, the string
      gets decompressed using the compressor framework when it gets fetch into main memory. 

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
