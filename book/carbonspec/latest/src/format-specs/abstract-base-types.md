# Abstract Base Types

Annotation of container to mark a abstract type is done by replacing the containers begin marker with a special marker (called *derivation marker*) that declares the container as a particular abstract type. 

For instance replacing the object container begin marker `[{]` by the derivation marker `[~]` declares the object container as `sorted-mulimap`. 

Per definition, the deriviation marker for abstract base types is the begin marker for that container type. Therefore, abstract base types must not be annotated. In simpler words with an example, `[{]` always marks an object container that is an `unsorted-multimap` (see below). 

Carbon supports the following two abstract base types:

- unsorted multi set (`unsorted-multiset`)
- unsorted multi map (`unsorted-multimap`)

