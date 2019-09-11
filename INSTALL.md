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

By default, all targets are built in *debug* configuration and without *source amalgamation*. Source amalgamation is
a technique that combines all source file of into one translation unit to improve runtime performance by enhance 
compiler optimizations. You find source amalgamation in database system SQLite, the JavaScript interpreter Duktape,
and others. The downside of source amalgamation is more cumbersome debugging because a typical debugger is not optimized
for such large source files.

To build targets in *release mode*, define the option `-DBUILD_TYPE=Release` for `cmake`. To turn source amalgamation
on, define the option `-DUSE_AMALGAMATION=ON` for `cmake`. 

For development, use
```
$ cmake -DBUILD_TYPE=Debug -DUSE_AMALGAMATION=OFF . && make -j4 && make test && sudo make install
```
and for deployments, use 
```
$ cmake -DBUILD_TYPE=Release -DUSE_AMALGAMATION=ON . && make -j4 && make test && sudo make install
```
 

The carbon library internally tracks some 
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

After a successful build, the tool is located in the `bin` directory. 
The tool supports the POSIX standard for its arguments. Type `bin/jakson-tool` for usage instructions.

In case `make install` was called, 
```
$ jakson-tool
```
 
is enough to execute the tool.