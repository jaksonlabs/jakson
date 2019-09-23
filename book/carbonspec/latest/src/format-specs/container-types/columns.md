# Columns

Carbon Type    | Description                      | Size                | *null*-Value | Begin Marker | Abstract Type
---------------|----------------------------------|---------------------|--------------|--------------|---------
`column`       | list of fixed-typed elements     | 3<sup>+</sup> bytes | `null` value | *see below* | `unsorted-multiset` 

## Encoding

```
[<column-marker>](num-of-elems)(cap-of-elems) <values>...
```

### Column Types

Marker | Container | Description             | Size     | *null*-Value       
--------|----------|-------------------------|----------|--------------------
`[B]`  | `column-boolean` | three-valued logic      | 1 B each | 2			       
`[1]`  | `column-u8` | unsigned 8-bit int      | 1 B each | 2<sup>8</sup> - 1  
`[2]`  | `column-u16` | unsigned 16-bit int     | 2 B each | 2<sup>16</sup> - 1 
`[3]`  | `column-u32` | unsigned 32-bit int     | 4 B each | 2<sup>16</sup> - 1 
`[4]`  | `column-u64` | unsigned 64-bit int     | 8 B each | 2<sup>16</sup> - 1 
`[5]`  | `column-i8` | signed 8-bit int        | 1 B each | -2<sup>7</sup>     
`[6]`  | `column-i16` | signed 16-bit int       | 2 B each | -2<sup>15</sup>    
`[7]`  | `column-i32` | signed 32-bit int       | 4 B each | -2<sup>31</sup>    
`[8]`  | `column-i64` | signed 64-bit int       | 8 B each | -2<sup>63</sup>    
`[R]`  | `column-float` | 32-bit float            | 4 B each | `NAN` value 	   

<span class="caption">Table C-1: Carbon `column` Type Support and Markers</span>

### Values

```
<values>
```

### Capacities

```
...
```

#### Abstract Type

A `column` is one [abstract base type](../abstract-types.md) for [`unsorted-multiset`](../abstract-base-types.md). The other base type is `array`, see [array containers](arrays.md).

### Example


JSON snippet
```json
[ -4, 2, null ]
```

A (compacted) Carbon `column-i8`.

```
[5] (3)(3) [-4] [2] [-128]
```

### Record Container Compatibility

> Note that a `column` container cannot replace a `record` container. If the input Json string is a plain Json array, this array is always stored as `record` (i.e., an array) rather than a column. To force a `column` container, the input Json array must be wrapped with an object.

JSON file
```json
[ 1, 2, 3 ]
```

A (compacted) Carbon `record` with no primary key encoding the JSON file from above as `array` container.

```
[nokey][[] [1][2][3] []] 
```
