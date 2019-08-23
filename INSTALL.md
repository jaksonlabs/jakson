libcarbon uses [CMake](https://cmake.org) as build system. CMake 3.9.6 or higher is required. 
For testing, [Google Test Framework](https://github.com/google/googletest) is required. You will find
installation instructions in [misc/INSTALL_GTEST.md](misc/INSTALL_GTEST.md). Libcarbon is built for the [Clang C Compiler](https://clang.llvm.org). 
To install this compiler on Linux, use `$ sudo apt install clang` if not yet present and set
`clang` as standard compiler by typing `$ sudo update-alternatives --config cc` and 
`$ sudo update-alternatives --config c++`.  

The basic usage is 
```
$ cmake . && make -j4 && make test && sudo make install
```

By default, all targets are built as release configuration. To build all targets in *debug mode* (without compiler 
optimization, with debug symbols enabled, and other debug-related features available), use 
`cmake -DBUILD_TYPE=Debug .`. The carbon library internally tracks some 
information in a log. To To turn on trace, information, warn or debug log in debug mode, set the options
`-DLOG_TRACE=on`, `-DLOG_INFO=on`, `-DLOG_WARN=on`, and `-DLOG_DEBUG=on` for `cmake`. Hence, to turn on debug mode
with all logs, use `cmake -DBUILD_TYPE=Debug -DLOG_TRACE=on -DLOG_INFO=on -DLOG_WARN=on -DLOG_DEBUG=on .`.


A tool to work with Carbon files (called `jakson-tool`) is shipped with this library.
This tool is automatically built when `make` is called, and installed via `make install`. However, to build the tool 
via its target, type 

```
$ cmake .
$ make jakson-tool
```

After a successful build, the tool is located in the `build` directory. 
The tool supports the POSIX standard for its arguments. Type `build/jakson-tool` for usage instructions.

In case `make install` was called, 
```
$ jakson-tool
```
 
is enough to execute the tool.