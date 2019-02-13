## 0.1.00.05 [2019-02-XX]
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

## 0.1.00.00 [2019-02-07]

- Initial standalone release
- Set license to MIT
- Refactoring and cleanups