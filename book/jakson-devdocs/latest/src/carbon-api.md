# Carbon API

> **This document is work-in-progress**. 

> You see the latest version in the `master` branch, as of 16th September 2019.

A core feature in Jakson is its way to organize, store, and access self-describing ("semi-structured") data. For that purpose, Jakson contains an implementation of the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org) in its core. 

The Jakson Carbon API builds, parses, and queries Carbon files with full compatibility to [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org), and the expressiveness of [JSON (RFC 8259)](https://tools.ietf.org/html/rfc8259). 

This chapter is about the Carbon implementation inside Jakson, and how its API is used for Jakson or any third party project. In particular, this chapter contains the following content:

- [Section 1](carbon-api/carbon-overview.md), *Carbon Overview*, summarizes the properties of the record format Carbon
- [Section 2](carbon-api/project-setup.md), *Project Setup*, provides a step-by-step tutorial on how to get the Carbon API running for both Jakson and third-party project
- [Section 4](carbon-api/construct-carbon-records.md), *Carbon Records*, is about Carbon records, container types, and construction of these by several ways, including parsing from Json and manual construction
- [Section 5](carbon-api/browse-contents.md), *Browse Contents*, explains value types and classes, and shows the usage of array, object and column iterators and how they are used to navigate through a record
- [Section 6](carbon-api/find-contents.md), *Find Contents*, centers around fetching values and containers if the particular path (expressed as dot-notated path) is known
- [Section 7](carbon-api/modify-contents.md), *Modify Carbon Records*, gives an overview on the revision concept of Carbon records and their usage, as well as shows tutorials on updating, inserting, and deletion of fields and properties in Carbon Records
- [Section 8](carbon-api/record-identification.md), *Record Identification*, deals with the API to specify and query an records identity, which is control on a (user-defined or auto-generated) record-local primary key
- [Section 9](carbon-api/record-optimization.md), *Record Optimization*, goes into details of optimization flags for Carbon records, which are about handling of (reserved, optional) memory capacities in Carbon Records, or about index creation
- [Section 10](carbon-api/conversion-to-json.md), *Conversion to JSON*, explains how to use Carbon formatters to print a records into a certain format, such as JSON with or without additional meta data
- [Section 11](carbon-api/advanced-topics.md), *Advanced Topics*, considers lower-level operations related to JSON parsing, and dot-path object construction, character strings and user-defined data types

Please note that the Carbon implementation is improved from version to version. It may be the case that you find inconsitency, although we try our best to keep the documentation and sources in sync. In case of any weird or out-dated content, please [contact the project owner](mailto:pinnecke@ovgu.de).

## License

This developer documentation, all the example source code, is released under the [MIT Licsense](https://github.com/jaksonlabs/jakson/blob/master/LICENSE).

