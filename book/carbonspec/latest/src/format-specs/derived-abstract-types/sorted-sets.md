# Sorted Set

| Abstract Type   | Elements | Distinct    | Sorted |
|-----------------|----------|-------------|--------|
| `sorted-set`    | values   | yes         | yes    |

## Based on `array` Containers

| Container Type | Marker | Derivation   
|----------------|--------|------------
| `array`        | `[[]`  | `[=]`               

## Based on `column` Containers

| Container Type   | Marker | Derivation   
|------------------|--------|------------
| `column-u8`      | `[1]`  | `[ETX]`               
| `column-u16`     | `[2]`  | `[BEL]`               
| `column-u32`     | `[3]`  | `[VT]`               
| `column-u64`     | `[4]`  | `[S1]`               
| `column-i8`      | `[5]`  | `[DC3]`               
| `column-i16`     | `[6]`  | `[ETB]`               
| `column-i32`     | `[7]`  | `[ESC]`               
| `column-i64`     | `[8]`  | `[US]`               
| `column-float`   | `[R]`  | `[.]`               
| `column-boolean` | `[B]`  | `[DEL]`               

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

A (compacted) Carbon `sorted-set` for the JSON snippet from above, encoding the JSON array with the base type `array` holding two `u8`, and one `null` values.

```
[=] 
	[n] 
	[c][2] 
	[c][4] 	
[]] 
```

### Encoding with Base Type `columns`

A (compacted) Carbon `sorted-set` for the JSON snippet from above, encoding the JSON array with the base type `column` holding two `u8`, and one `null` value.

```
[ETX] 
	(5)(5) 
	[2]	
	[4]
	[255]
```

