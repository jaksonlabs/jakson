# Sorted Multi Sets

A sorted multi set is a sequence of elements with a particular sorting, which may contain duplicates.

| Abstract Type       | Elements | Distinct    | Sorted |
|---------------------|----------|-------------|--------|
| `sorted-multiset`   | values   | no          | yes    |

## Based on `array` Containers

| Container Type | Marker | Derivation   
|----------------|--------|------------
| `array`        | `[[]`  | `[<]`               

## Based on `column` Containers

| Container Type   | Marker | Derivation   
|------------------|--------|------------
| `column-u8`      | `[1]`  | `[SOH]`               
| `column-u16`     | `[2]`  | `[ENQ]`               
| `column-u32`     | `[3]`  | `[TAB]`               
| `column-u64`     | `[4]`  | `[CR]`               
| `column-i8`      | `[5]`  | `[DC1]`               
| `column-i16`     | `[6]`  | `[NAK]`               
| `column-i32`     | `[7]`  | `[EM]`               
| `column-i64`     | `[8]`  | `[GS]`               
| `column-float`   | `[R]`  | `["]`               
| `column-boolean` | `[B]`  | `[_]`               

## Example

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

A (compacted) Carbon `sorted-multiset` for the JSON snippet from above, encoding the JSON array with the base type `array` holding three `u8`, and two `null` values.

```
[<] 
	[n] 
	[n] 
	[c][2]
	[c][2]
	[c][4] 
[]] 
```

### Encoding with Base Type `columns`

A (compacted) Carbon `sorted-multiset` for the JSON snippet from above, encoding the JSON array with the base type `column` holding three `u8`, and two `null` values.

```
[SOH] 
	(5)(5) 
	[255] 
	[255] 	
	[2] 
	[2] 
	[4] 	
```