# Unsorted Set

| Abstract Type   | Elements | Distinct    | Sorted |
|-----------------|----------|-------------|--------|
| `unsorted-set`  | values   | yes         | no     |

## Based on `array` Containers

| Container Type | Marker | Derivation   
|----------------|--------|------------
| `array`        | `[[]`  | `[/]`               

## Based on `column` Containers

| Container Type   | Marker | Derivation   
|------------------|--------|------------
| `column-u8`      | `[1]`  | `[STX]`               
| `column-u16`     | `[2]`  | `[ACK]`               
| `column-u32`     | `[3]`  | `[LF]`               
| `column-u64`     | `[4]`  | `[S0]`               
| `column-i8`      | `[5]`  | `[DC2]`               
| `column-i16`     | `[6]`  | `[SYN]`               
| `column-i32`     | `[7]`  | `[SUB]`               
| `column-i64`     | `[8]`  | `[RS]`               
| `column-float`   | `[R]`  | `[$]`               
| `column-boolean` | `[B]`  | `[']`               


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

A (compacted) Carbon `unsorted-set` for the JSON snippet from above, encoding the JSON array with the base type `array` holding two `u8`, and one `null` value.

```
[/] 
	[c][4] 
	[c][2] 
	[n] 
[]] 
```


### Encoding with Base Type `columns`

A (compacted) Carbon `unsorted-set` for the JSON snippet from above, encoding the JSON array with the base type `column` holding two `u8`, and one `null` value.

```
[STX] 
	(5)(5) 
	[4]
	[2]
	[255]
```


