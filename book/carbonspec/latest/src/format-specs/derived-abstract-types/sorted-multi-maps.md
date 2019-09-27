# Sorted Multi Maps

| Abstract Type     | Elements        | Container Type | Marker | Derivation | Distinct | Sorted |
|-------------------|-----------------|----------------|--------|------------|----------|--------|
| `sorted-multimap` | key-value pairs | `object`       | `[{]`  | `[~]`      | no       | yes    |


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

### Carbon Encoding

A (compacted) Carbon `sorted-multimap` for the JSON snippet from above, encoding the JSON object with the base type `object` holding four string_buffer properties.

```
[~] 
	(1)[a] [s](1)[b]
	(1)[a] [s](1)[c]
	(1)[x] [s](1)[y]
	(1)[x] [s](1)[y]
[}]
```