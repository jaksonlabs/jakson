# Arrays

Carbon Type    | Description                      | Size                | *null*-Value | Marker
---------------|----------------------------------|---------------------|--------------|------------
`array`        | list of variable-typed elements  | 2<sup>+</sup> bytes | `null` value | `[`, `]`

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
binary, custom binary, null, array, column 
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