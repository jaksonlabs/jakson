# Unsorted Multi Set

| Abstract Type       | Derivation Marker   | Container Type       | Elements | Distinct    | Sorted |
|---------------------|---------------------|----------------------|----------|-------------|--------|
| `unsorted-multiset` | *container marker*  |`array`, `column`     | values   | no          | no     |

The abstract base type **unsorted multi set** (`unsorted-multiset`) is a sequence of elements (potentially with duplicates). This sequence has no special sorting its elements.

## Example


### JSON Snippet
```json
[ 
	4, 
	2, 
	null, 
	null, 
	2 
]
```

### Encoding with Base Type `array`

A (compacted) Carbon `unsorted-multiset` for the JSON snippet from above, encoding the JSON array with the base type `array` holding three `u8`, and two `null` values.

```
[[] 
	[c][4] 
	[c][2] 
	[n] 
	[n] 
	[c][2] 
[]] 
```

### Encoding with Base Type `column-u8`

A (compacted) Carbon `unsorted-multiset` for the JSON snippet from above, encoding the JSON array with the base type `column-u8` holding three `u8`, and two `null` values.

```
[1] 
	(5)(5) 
	[4] 
	[2] 
	[2] 
	[255] 
	[255] 
```
