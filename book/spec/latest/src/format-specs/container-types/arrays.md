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