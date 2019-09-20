# Arrays

Type    | Description                      | Size                | *null*-Value | Marker | Abstract Type
---------------|----------------------------------|---------------------|--------------|----------|--
`array`        | variable-typed list  | 2<sup>+</sup> bytes | `null` | `[`, `]` | `unsorted_list`

## Encoding

```
[[] <values> ...[]]
```

### Values

```
<values>
```

#### Type Support

```
true, false, u8, u16, u32, u64, i8, i16, i32, i64, float, string, 
binary, custom binary, null, array, column-u8, column-u16, column-u32, 
column-u64, column-i8, column-i16, column-i32, column-i64, column-float, 
column-boolean, object 
```

### Capacities

```
...
```


#### Abstract Type

An `array` is one [abstract type](../abstract-types.md) for the [`unsorted_list`](../abstract-base-types.md) abstract base type. The other base type is `column`, see [column containers](columns.md).

### Example


JSON snippet
```json
[ "The", "Number", 23 ]
```

A (compacted) Carbon `array` of two `string` values and one `u8` value.

```
[[] [s](3)[The] [s](6)[Number] [c][23] []] 
```