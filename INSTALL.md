libcarbon uses [CMake](https://cmake.org) as build system. CMake 3.9.6 or higher is required.

The basic usage is 
```
cmake .
make
make install
```
After installation, link against `libcarbon` and use
 
```#include <carbon/carbon.h>```

in your project in order to use the library.

A tool to work with CARBON files (called `carbon-tool`) is shipped with this library.
The build process is 
```
cmake .
make carbon-tool
```
After a successful build, the tool is located in the `build` directory.

Examples files are located in the `examples` directory, and are build with
```
cmake .
make examples-${name}
```
where `${name}`  is the topic (e.g., `error`). See source files in the `examples` 
directory to get a list of possible targets.