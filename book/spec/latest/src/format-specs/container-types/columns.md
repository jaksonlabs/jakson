# Columns

Carbon Type    | Description                      | Size                | *null*-Value | Begin Marker 
---------------|----------------------------------|---------------------|--------------|--------------
`column`       | list of fixed-typed elements     | 3<sup>+</sup> bytes | `null` value | *see below*  

## Encoding

```
[<column-marker>](num-of-elems)(cap-of-elems) <values>...
```

### Column Types

Carbon Type  | Marker | Description             | Size     | *null*-Value       
-------------|--------|-------------------------|----------|--------------------
`boolean`    | `[B]`  | three-valued logic      | 1 B each | 2			       
`u8`         | `[1]`  | unsigned 8-bit int      | 1 B each | 2<sup>8</sup> - 1  
`u16`        | `[2]`  | unsigned 16-bit int     | 2 B each | 2<sup>16</sup> - 1 
`u32`        | `[3]`  | unsigned 32-bit int     | 4 B each | 2<sup>16</sup> - 1 
`u64`        | `[4]`  | unsigned 64-bit int     | 8 B each | 2<sup>16</sup> - 1 
`i8`         | `[5]`  | signed 8-bit int        | 1 B each | -2<sup>7</sup>     
`i16`        | `[6]`  | signed 16-bit int       | 2 B each | -2<sup>15</sup>    
`i32`        | `[7]`  | signed 32-bit int       | 4 B each | -2<sup>31</sup>    
`i64`        | `[8]`  | signed 64-bit int       | 8 B each | -2<sup>63</sup>    
`float`      | `[R]`  | 32-bit float            | 4 B each | `NAN` value 	   

<span class="caption">Table C-1: Carbon `column` Type Support and Markers</span>

### Values

```
<values>
```

### Capacities

```
...
```

### Example


JSON snippet
```json
[ "The", "Number", 23 ]
```

A (compacted) Carbon file, which encodes the JSON array as `array` of two `string` values and one `u8` value.

```
[[] [s](3)[The] [s](6)[Number] [c][23] []] 
```