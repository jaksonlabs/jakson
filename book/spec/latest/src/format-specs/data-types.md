# Data Types

Carbon Type  | Description             | Size                | *null*-Value      | Marker 
-------------|-------------------------|---------------------|-------------------|------------------
`true`       | truth value *true*  | 0 byte              | `null`            | `[t]`
`false`      | truth value *false* | 0 byte              | `null`            | `[f]`
`boolean`    | three-valued logic    | 0<sup>+</sup> bytes | const | `[B]`
`u8`         | unsigned 8-bit int  | 1 B | `null`, 2<sup>8</sup> - 1  | `[c]`, `[1]`
`u16`        | unsigned 16-bit int | 2 B | `null`, 2<sup>16</sup> - 1 | `[d]`, `[2]`
`u32`        | unsigned 32-bit int | 4 B | `null`, 2<sup>16</sup> - 1 | `[i]`, `[3]`
`u64`        | unsigned 64-bit int | 8 B | `null`, 2<sup>16</sup> - 1 | `[l]`, `[4]`
`i8`         | signed 8-bit int    | 1 B | `null`, -2<sup>7</sup>     | `[C]`, `[5]`
`i16`        | signed 16-bit int   | 2 B | `null`, -2<sup>15</sup>    | `[D]`, `[6]`
`i32`        | signed 32-bit int   | 4 B | `null`, -2<sup>31</sup>    | `[I]`, `[7]`
`i64`        | signed 64-bit int   | 8 B | `null`, -2<sup>63</sup>    | `[L]`, `[8]`
`float`      | 32-bit float | 4 byte  | `null`,  `NAN` | `[r]`, `[R]` 
`string`        | string of `n` chars | `n` bytes | `null`       | `[s]`
`binary`        | binary of `n` bytes | `n` + 2<sup>+</sup> bytes | `null`       | `[b]`
`custom binary` | binary of `n` bytes | `n` + 3<sup>+</sup> bytes | `null`       | `[x]`
`null`      | absence of data | 0<sup>+</sup> byte | `null` | `[n]`
