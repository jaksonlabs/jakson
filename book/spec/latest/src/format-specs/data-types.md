# Data Types

Carbon Type  | Description             | Size                | *null*-Value      | Marker 
-------------|-------------------------|---------------------|-------------------|------------------
`true`       | truth value *true*  | 0 byte              | `null`            | `[t]`
`false`      | truth value *false* | 0 byte              | `null`            | `[f]`
`boolean`    | three-valued logic    | 0<sup>+</sup> bytes | *const* | *const*
`u8`         | unsigned 8-bit int  | 1 B | `null`, 2<sup>8</sup> - 1  | `[c]`
`u16`        | unsigned 16-bit int | 2 B | `null`, 2<sup>16</sup> - 1 | `[d]`
`u32`        | unsigned 32-bit int | 4 B | `null`, 2<sup>16</sup> - 1 | `[i]`
`u64`        | unsigned 64-bit int | 8 B | `null`, 2<sup>16</sup> - 1 | `[l]`
`i8`         | signed 8-bit int    | 1 B | `null`, -2<sup>7</sup>     | `[C]`
`i16`        | signed 16-bit int   | 2 B | `null`, -2<sup>15</sup>    | `[D]`
`i32`        | signed 32-bit int   | 4 B | `null`, -2<sup>31</sup>    | `[I]`
`i64`        | signed 64-bit int   | 8 B | `null`, -2<sup>63</sup>    | `[L]`
`float`      | 32-bit float | 4 byte  | `null`,  `NAN` | `r` 
`string`        | string of `n` chars | `n` bytes | `null`       | `[s]`
`binary`        | binary of `n` bytes | `n` + 2<sup>+</sup> bytes | `null`       | `[b]`
`custom binary` | binary of `n` bytes | `n` + 3<sup>+</sup> bytes | `null`       | `[x]`
`null`      | absence of data | 0<sup>+</sup> byte | `null` | `[n]`,  *const*
