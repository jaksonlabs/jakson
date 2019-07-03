# Integer Numbers

Type  | Description             | Size    | Min                 | Max                | *null*-Value
------|-------------------------|---------|---------------------|--------------------|--------------------
`u8`  | unsigned 8-bit integer  | 1 bytes | 0                   | 2<sup>8</sup> - 2  | 2<sup>8</sup> - 1
`u16` | unsigned 16-bit integer | 2 bytes | 0                   | 2<sup>16</sup> - 2 | 2<sup>16</sup> - 1
`u32` | unsigned 32-bit integer | 4 bytes | 0                   | 2<sup>16</sup> - 2 | 2<sup>16</sup> - 1
`u64` | unsigned 64-bit integer | 8 bytes | 0                   | 2<sup>16</sup> - 2 | 2<sup>16</sup> - 1
`i8`  | signed 8-bit integer    | 1 byte  | −2<sup>7</sup> + 1  | 2<sup>7</sup> - 2  | -2<sup>7</sup>
`i16` | signed 16-bit integer   | 2 bytes | −2<sup>15</sup> + 1 | 2<sup>15</sup> - 2 | -2<sup>15</sup>
`i32` | signed 32-bit integer   | 4 bytes | −2<sup>31</sup> + 1 | 2<sup>31</sup> - 2 | -2<sup>31</sup>
`i64` | signed 64-bit integer   | 8 bytes | −2<sup>63</sup> + 1 | 2<sup>63</sup> - 2 | -2<sup>63</sup>


## As Field Value


Description              | Size      | Marker          | Payload
-------------------------|-----------|-----------------|---------------
 unsigned 8-bit integer  | 1+1 bytes | `[c]` (char)    | an `u8` value
 unsigned 16-bit integer | 1+2 bytes | `[d]` (decimal) | an `u16` value
 unsigned 32-bit integer | 1+4 bytes | `[i]` (integer) | an `u32` value
 unsigned 64-bit integer | 1+8 bytes | `[l]` (long)    | an `u64` value
 signed 8-bit integer    | 1+1 bytes | `[C]` (char)    | an `i8` value
 signed 16-bit integer   | 1+2 bytes | `[D]` (decimal) | an `i16` value
 signed 32-bit integer   | 1+4 bytes | `[I]` (integer) | an `i32` value
 signed 64-bit integer   | 1+8 bytes | `[L]` (long)    | an `i64` value


### Example

JSON snippet
```json
[-42, 42]
```

A (compacted) Carbon file, which encodes the JSON array as `array` of one `i8` and one `u8` integer.

```
[[] [c][-42] [C][42] []]
```

## As Column Value


Column Type Marker               | Marker Size | Element Size | Block
---------------------------------|-------------|--------------| ---------------
 `[c]`, unsigned 8-bit integers  | 1 byte      | 1 byte each  | an `u8` value
 `[d]`, unsigned 16-bit integers | 1 byte      | 2 bytes each | an `u16` value
 `[i]`, unsigned 32-bit integers | 1 byte      | 4 bytes each | an `u32` value
 `[l]`, unsigned 64-bit integers | 1 byte      | 8 bytes each | an `u64` value
 `[C]`, signed 8-bit integers    | 1 byte      | 1 byte each  | an `i8` value
 `[D]`, signed 16-bit integers   | 1 byte      | 2 bytes each | an `i16` value
 `[I]`, signed 32-bit integers   | 1 byte      | 4 bytes each | an `i32` value
 `[L]`, signed 64-bit integers   | 1 byte      | 8 bytes each | an `i64` value



### Example

JSON snippet
```json
[-42, 42]
```

A (compacted) Carbon file, which encodes the JSON array as `i8` column.

```
[(] [C][2][2] [-42][42] [)]
```
