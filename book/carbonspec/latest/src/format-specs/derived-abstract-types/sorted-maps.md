# Sorted Maps

| Abstract Type   | Elements          | Container Type | Marker | Derivation | Distinct | Sorted |
|-----------------|-------------------|----------------|--------|------------|----------|--------|
| `sorted-map`    | key-value pairs   | `object`       | `[{]`  | `[#]`      | yes      | yes    |


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

A (compacted) Carbon `sorted-map` for the JSON snippet from above, encoding the JSON object with the base type `object` holding two string properties (assuming `"a":"c"` was added after `"a":"b"`).

```
[#] 
	(1)[a] [s](1)[c]	
	(1)[x] [s](1)[y]
[}]
```