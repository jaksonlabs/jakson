# Columnar Binary JSON Books

This directory (`book`) contains the Carbon Specification document, and a manual on the [libcarbon](https://github.com/protolabs/libcarbon) library.

## Specification 

The sources of current Carbon Specification draft are located in the `spec/latest` directory. The specification is formatted with  markdown, and built with [MdBook](https://github.com/rust-lang-nursery/mdBook).

To build the specification book, type in your bash
```
$ cd spec/latest
$ mdbook build
```

## Libcarbon Manual

A set of tutorial and documentations to the official [libcarbon](https://github.com/protolabs/libcarbon) implementation are located as a [MdBook](https://github.com/rust-lang-nursery/mdBook) book in the `libcarbon` directory. The lastest version is located in `libcarbon/latest`.

To build the manual book, type in your bash
```
$ cd libcarbon/latest
$ mdbook build
```