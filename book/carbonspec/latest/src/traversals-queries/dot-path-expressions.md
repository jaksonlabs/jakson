# Dot Path Expressions

A dot path expression is used to access a value within a Carbon `record` by describing a path to this value.

A dot path expression neither modifies a `record` nor supports any kind of predicates beyond those on paths.
In simpler words, the intention of a dot path expression is express the path to a particular value not querying
for paths given some contraints to values. 

## Comparison to Dot Notation as in MongoDB or CouchDB

Dot path expression are a superset of value paths that be expression via a "dot notation" in MongoDB or CouchDB.
In addition to proving a fixed path to a value as one would do using a dot notation, a dot path expression allows
to give a more fuzzy description of the path. Directly speaking, when using the dot notation, there is no way
to access values if the path to these values cannot be described completely. This is a strong limitation for
explorative queries in structures that are only partly known. In MongoDB or CouchDB one would typically use the
aggregation pipeline or a MapReduce job for these purposes. Carbon dot path expressions allow to query for
a value where there is some uncertainty about its actual path.

In a nutshell, dot path expressions allow the following capabilities beyond the dot notation:
- ...
- Clear difference whether a path results in value that is annotated as not present (i.e., a `null` is returned), and
  a path that does not resolve to any value (i.e., a `undef` is returned)

## Comparison to JSON/SQL Path Expressions

JSON/SQL Path Expressions is part of the SQL:2016 Standard and allows 


## Grammar


```
dot-path-unit           ::= path-expression ('.' path-expression)*
path-expression         ::= array-accessor | property-accessor 
array-accessor          ::= [0] | [1-9][0-9]* 
property-accessor       ::= field-name | '"' string '"' | key-predicate
key-predicate           ::= '$' function-name '(' function-parameter-list ')'
field-name              ::= [a-zA-Z]('\"' | [a-zA-Z0-9])*
function-name           ::= [a-zA-Z][a-zA-Z0-9]*
function-parameter-list ::= non-strict-json-string
```


## Property Access

JSON snippet
```json
{
   "title":"Back to the Future",
   "sub-title":null,
   "year":1985,
   "imdb-rating":8.5,
   "meta": { "keywords": [
      			"time travel",
      			"delorean",
      			"comedy"
   			 ],
             "personal comment": "must see"
   },
   "release-dates":[
      1985, 1986, 1987,
      1992, 2008, 2010,
      2012, 2015, 2016
   ]
}
```

### Key-Based Access

Dot Path
```
title
```

Result (Json)
```
"Back to the Future"
```


### Nested Property Accesss

Dot Path
```
meta.keywords
```

Result (Json)
```
["time travel", "delorean", "comedy"]
```

### Non-Identifier Key-Based Access

Dot Path
```
meta.keywords."personal comment"
```

Result (Json)
```
"must see"
```

## Access to Values via Uncertain Paths

### Key-Based Fuzzy Paths

Dot Path
```
$ends_with(name: "le", ignore_case: true)"
```

Result (Json)
```json
[
    {
    	"value": "Back to the Future",
    	"type": "string",
    	"path": "title"
    },
    {
    	"value": null,
    	"type": "null",
    	"path": "sub-title"
    }
]
```

The specification defines a minium set of functions that must be supportet for dot path expression strings. 
See Section [Dot Path Functions](traversals-queries/path-functions.md) for more details.



## Array Element Access

JSON snippet
```json
[
	{
	   "title":"Back to the Future",
	   "year":1985,
	   "keywords": [
      			"time travel",
      			"delorean",
      			"comedy"
   			 ]
	}, 
	{
	   "title":"Back to the Future Part II",
	   "year":1989
	},
	[
	   "time travel", "delorean", "comedy"
	]
]
```

### Element Access

Dot Path
```
1.title
```

Result (Json)
```
"Back to the Future Part II"
```

### Nested Array Element Access

Dot Path
```
2.0
```

Result (Json)
```
"time travel"
```


## Mixed Array and Property Access

Dot Path
```
0.keywords.2
```

Result (Json)
```
"comedy"
```

### Array Element Fuzzy Paths

Dot Path
```
$tail(skip: 1).title"
```

Result (Json)
```json
[
    {
    	"value": "Back to the Future Part II",
    	"type": "string",
    	"path": "1.title"
    }
]
```

The specification defines a minium set of functions that must be supportet for dot path expression strings. 
See  Section [Dot Path Functions](traversals-queries/path-functions.md) for more details.

### General Fuzzy Paths and Conditional Branching

See Section [Dot Path Functions](traversals-queries/path-functions.md).

### Unresolvable Paths

If a (synatic correct) path does not resolve to a value, the value `undef` is returned. This applies to paths that do not point to a value, 
or paths that cannot be fully traversed since a non-container type occurs during the path evaluation.

Input Json
```json
{
  "x": "y",
  "z": [1, 2, 3]
}
```

The dot path expression `y` will return `undef` since there is no key named `"y"` in the input Json.
The dot path expression `z.1.5` will return `undef` since the path evaluation must be aborted with `z.1` since the element `2` is not a traversable container.

### Note

Internally, Carbon organizes data in a record container which is basically an array. Therefore, 

JSON snippet
```json
{ "x": "y" }
```

is stored as `[{ ... }]`. Hence dot path expression `title` is equivalent to `0.title`; both result in `"y"`. Actually, omitting
`0.` in this context is syntactig suggar: omitting the index `0` is possible as long as the input Json is an Json object. 

In cases where the input is not a single Json object, omitting the first array index is not valid.

JSON snippet
```json
[{ "x": "y" }]
```

the dot path expression `0.x` will result in `"y"` as expected, but the dot path expression `x` will result in a carbon `undef`. The
reason for that is to guarantee container semantics as given by the user input. 

JSON snippet
```
A = { "x": "y" }
B = [{ "x": "y"}, { "x": "z"}]
```

`A` is an object, and `B` is an array. Allowing omitting of `0.` to access the property `x` in `B` would result in `"y"` as expected 
but would imply `B` is an object like `A`, which is not the case. The intention of omitting the index `0` in dot path expressions
for Json inputs that consits of an single Json object only, is avoid forcing semantics (i.e., that a record is an array and never be
a single object in Carbon) that are design-specific for Carbon. However, omitting this index in all other cases would change the 
user input data at least implictly (such as treating an array of objects like a single object). 

