# Identification

```
<primary-key>
```

```
<type-marker> <value-type>
```

Carbon Type | Description                                  | Size   | Type Marker | Value Type
------------|----------------------------------------------|--------|-------------|-------
`nokey`  | no identifier assigned    | 0 B | `[?]` | *none*
`autokey`  | 64-bit auto-generated unsigned identifier    | 8 B | `[*]` | `u64`
`ukey`   | 64-bit user-defined unsigned identifier      | 8 B | `[+]` | `u64`
`ikey`   | 64-bit user-defined signed identifier        | 8 B | `[-]` | `i64`
`skey`   |`n`-char string user-def. identifier | `n` B | `[!]` | `string`

<span class="caption">Table FO-1: Record Primary Key Types and Encoding</span>

> The value generation of `autokey` is implementation-dependent.

> The default value for numeric keys (`autokey`, `ukey`, and `ikey`) is `0`. The empty string `""` is used to encode the default value for character string keys (`skey`). JSON formatters are required to format the value of `nokey` and the default value for character string keys as JSON `null`.

> If `nokey` is set, the `commit-hash` field for a carbon `record` is omitted. In any case of revison, the commit hash for such a `record` is `0`.

