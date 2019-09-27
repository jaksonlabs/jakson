# Jakson Developer Documentation

*By Marcus Pinnecke, with contributions from the Jakson community*

> **This document is work-in-progress**. 

> You see the latest version as of 20th September 2019.

Welcome to the *Jakson Developer Documentation*, which will help you to get started with the Jakson project at source level!. This document aims to provide a good starting point for new developers, and a reference for developers already active in the project. 

<p style="text-align: center;">
  <img src="jakson-logo-200x200.png" width="200px" />
</p>

This documentations covers the majority of Jaksons core data structures and function, the projects organization, design decisions made for particular scenarios, and some hints in the general usage. In particular, this documentation is structured as folows:

- [Chapter 1](fundamentals.md), *Fundamentals*, covers code style and system-wide fundamental structures. In particular, the chapter contains the err system used in Jakson, macros for err handling, memory managment, branch-prediction, data-prefetching, bit manipulations, diagnostics, and general utilities. In addition, this chapter describes memory blocks and memory files as Jaksons core abstractions for building highly cache-efficent complex objects in dynamic memory (called *flat objects*). Afterwards, the standard library for common data structures, such as vectors or bitmaps, and the extended library for data structures, such as variable-length encoded integers or flat columns, is covered. The chapters closes with an overview on built-in hash functions, and ulitiy modules including the convenience string_buffer-buffer type and low-level hexdumping of memory areas.

- [Chapter 2](carbon-api.md), *Carbon API*, covers the application programming interface (API) to Jakson implementation of the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org), and how this API is used inside Jakson and third-party projects. This API description includes construction of Carbon records by hand, from existing raw data, or by parsing from JSON. Here, the meaning and usage of different primary-key types is explained. Further, the chapter covers how data  records are queried by using an three-kind iterator model or by using a specialized find method that accepts dot-notated paths as character strings. Next, field typing, type handling, object property handling and abstractions are described. Afterwards, the record update policy and functionality is explained, which includes functions and semantics for insertions, deletions, and updates. A dedication section is about optimization strategies in Carbon, which center around the importance and meanings of around several areas of (optional) reserved memory inside records. Finally, this chapters describes different formatting options for Carbon files, such as conversion to plain-text JSON with or without Carbon-related meta data, and advanced topics including best practise on lower-level data structure for dot-notated paths, character strings and user defined types.

 - [Chapter 3](multi-threading.md), *Multi-Threading*, is about data structures built-in Jakson for both task and data parallel task execution. This include a brief description of Jaksons lightweight spinlock implementation, and the management of pooled parallel worker. A larger section is about the generic data parallelism framework, which contains functional-style primitives, including a parallel *for* function, a *map* operations, both *gather* and *scatter*, and a parallel *shuffle* operation. The framework additionally includes an optimized *filter* function that comes in two forms, a early and a late materializing flavor.

## License

This developer documentation, all the example source code, is released under the [MIT Licsense](https://github.com/jaksonlabs/jakson/blob/master/LICENSE).

