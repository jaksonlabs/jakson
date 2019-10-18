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
`null`      | absence of data (user-defined) | 0<sup>+</sup> byte | `null` | `[n]`
`undef`      | absence of data (structure-defined) | 0 byte | *none* | `[N]`

## Null and Undef
In carbon, there are two way to express absence of data, `null` and `undef`. The first one, `null`, is a
mapping of the Json constant `null`, and is used to express non-present data by the user, e.g., by the
Json string `"x": null`. In contrast, the second one, `undef` is used to express absence of data that is
not only annotated as non-present, but in fact not contained. In no cases, the user can express a `undef`
with its input, but will receive a `undef` (i.e., "not in list") value whenever the user `queries` for non-existing
data by a [dot path expression](traversals-queries/path-eval/path-eval.md).

The reason for differencing between `null` and `undef` is to enable clear semantic whether a particular
[dot path expression](traversals-queries/path-eval/path-eval.md) evaluates to a value that is stored
within the record and which is annotated with absence of data by the user (i.e., using a `null`),
or to a value that is not contained in the dataset (i.e., a `undef` is return).

To ephasis the differences and importance of `undef` and `null`, consider the following example

```json
{
    "x": null
}
```

Using a [dot path expression](traversals-queries/path-eval/path-eval.md) asking for the value of
the property `"x"` will result in `null` since `"x"` is a valid path that evaluates to a particular
value. In contrast, asking for the value of an non-existing property `"y"` will result in `undef` since
`"y"` does not exist. 