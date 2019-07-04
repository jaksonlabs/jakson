# Columns

Carbon Type    | Description                      | Size                | *null*-Value | Begin Marker | End
---------------|----------------------------------|---------------------|--------------|--------------|-----
`column`       | list of fixed-typed elements     | 4<sup>+</sup> bytes | `null` value | *see below*  | `#`

## Encoding

```
[<type>](num-of-elems)(cap-of-elems) <values>...
```

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