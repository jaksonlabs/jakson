# Columnar Binary JSON Books

This directory (`book`) contains the Carbon Specification document, and a developer manual for [Jakson](https://github.com/jaksonlabs/jakson) (including the Carbon API).

## Specification 

The sources of current Carbon Specification draft are located in the `carbonspec/latest` directory. 
The specification is formatted with  markdown, and built with [MdBook](https://github.com/rust-lang-nursery/mdBook).

To build the specification book, type in your bash
```
$ cd carbonspec/latest
$ mdbook build
```

> You find the latest stable snapshot on [carbonspec.org](http://www.carbonspec.org)

## Jakson Developer Documentation 

The sources of current Jakson Developer Documentation are located in the `jakson-devdoc/latest` directory. 
The Developer Documentation is formatted with  markdown, and built with [MdBook](https://github.com/rust-lang-nursery/mdBook).

To build the specification book, type in your bash
```
$ cd jakson-devdoc/latest
$ mdbook build
```

> You find the latest stable snapshot on [jakson-devdocs.org](http://www.jakson-devdocs.org)