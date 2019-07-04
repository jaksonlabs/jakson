# Identification

```
<primary-key>
```

```
<type-marker> <value-type>
```

Carbon Type | Description                                  | Size   | Type Marker | Value Type
------------|----------------------------------------------|--------|-------------|-------
`autokey`  | 64-bit auto-generated identifier    | 8 B | `[*]` | `u64`
`ukey`   | 64-bit user-defined unsigned identifier      | 8 B | `[+]` | `u64`
`ikey`   | 64-bit user-defined signed identifier        | 8 B | `[-]` | `i64`
`skey`   |`n`-char string user-def. identifier | `n` B | `[!]` | `string`

<span class="caption">Table FO-1: Record Primary Key Types and Encoding</span>

> The value generation of `autokey` is implementation-dependent.

For See more in section []().
