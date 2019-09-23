# Integer Numbers

Carbon  | Description              | Size    | Min                 | Max                | *null*-Value       | Marker
-------------|---------------------|---------|---------------------|--------------------|--------------------|-------
`u8`         | unsigned 8-bit int  | 1 B | 0                   | 2<sup>8</sup> - 2  | `null` or 2<sup>8</sup> - 1  | `[c]`
`u16`        | unsigned 16-bit int | 2 B | 0                   | 2<sup>16</sup> - 2 | `null` or2<sup>16</sup> - 1 | `[d]`
`u32`        | unsigned 32-bit int | 4 B | 0                   | 2<sup>16</sup> - 2 | `null` or2<sup>16</sup> - 1 | `[i]`
`u64`        | unsigned 64-bit int | 8 B | 0                   | 2<sup>16</sup> - 2 | `null` or2<sup>16</sup> - 1 | `[l]`
`i8`         | signed 8-bit int    | 1 B  | −2<sup>7</sup> + 1  | 2<sup>7</sup> - 2 | `null` or-2<sup>7</sup>     | `[C]`
`i16`        | signed 16-bit int   | 2 B | −2<sup>15</sup> + 1 | 2<sup>15</sup> - 2 | `null` or-2<sup>15</sup>    | `[D]`
`i32`        | signed 32-bit int   | 4 B | −2<sup>31</sup> + 1 | 2<sup>31</sup> - 2 | `null` or-2<sup>31</sup>    | `[I]`
`i64`        | signed 64-bit int   | 8 B | −2<sup>63</sup> + 1 | 2<sup>63</sup> - 2 | `null` or-2<sup>63</sup>    | `[L]`


## Encoding as Field Value



### Example

JSON snippet
```json
[-42, 42, null]
```

A (compacted) Carbon file, which encodes the JSON array as `array` of one `i8` and one `u8` value.

```
[[] [c][-42] [C][42] [n] []]
```

## Encoding as Column Value


### Example

JSON snippet
```json
[-42, 42, null]
```

A (compacted) Carbon `column-8`.

```
[C](3)(3) [-42][42][255]
```
