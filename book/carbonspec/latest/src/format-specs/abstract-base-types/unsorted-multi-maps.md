# Unsorted Multi Map

| Abstract Type       | Derivation Marker  | Container Type  | Elements | Distinct    | Sorted |
|---------------------|--------------------|-----------------|----------|-------------|--------|
| `unsorted-multimap` | *container marker* |`object`         | pairs    | no          | no     |

The abstract base type **unsorted multi map** (`unsorted_multimap`) is a sequence of key-value pairs (potentially with duplicate pairs, and duplicate keys). This sequence has no special sorting its elements.


## Example


### JSON Snippet

```json
{ 
	"x":"y", 
	"x":"y", 
	"a":"c",	
	"a":"b" 
}
```

### Encoding with Base Type `object`

A (compacted) Carbon `unsorted-multimap` for the JSON snippet from above, encoding the JSON object with the base type `object` holding four string_buffer properties.

```
[{] 
	(1)[x] [s](1)[y]
	(1)[x] [s](1)[y]
	(1)[a] [s](1)[c]	
	(1)[a] [s](1)[b]
[}]
```

> **JSON Standard Notes**. Note that [JSON (RFC 8259)](https://tools.ietf.org/html/rfc8259) does not restrict an object to unique keys or unique pairs. However, most JSON libraries reject objects with duplicate keys. For these libraries, use the [derived abstract type](derived-abstract-types.md) `sorted-map` or `unsorted-map`.