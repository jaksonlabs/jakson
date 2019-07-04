# Marker Format

```
[marker]
```

Carbon Type  | Description                                                 | Size   | Examples     
-------------|-------------------------------------------------------------|--------|-------------------
`marker`     | constant for encoding of items and containers  | 1 byte | `[n]`, `[[]`, `[x]` 


## Reserved Marker

Marker  | Carbon Type      | Descriptions
--------|------------------|---------------------------------------------
`[t]`   | `true`           | a field truth value *true*
`[f]`   | `false`          | a field truth value *false*
`[c]`   | `u8`             | an unsigned 8-bit integer field
`[d]`   | `u16`            | an unsigned 16-bit integer field
`[i]`   | `u32`            | an unsigned 32-bit integer field
`[l]`   | `u64`            | an unsigned 64-bit integer field
`[C]`   | `u8`             | a signed 8-bit integer field
`[D]`   | `u16`            | a signed 16-bit integer field
`[I]`   | `u32`            | a signed 32-bit integer field
`[L]`   | `u64`            | a signed 64-bit integer field
`[r]`   | `float`          | a single-precision floating number field
`[s]`   | `string`         | a character string field
`[b]`   | `binary`         | a binary string field with known MIME type
`[x]`   | `custom binary`  | a binary string field with user-defined type
`[n]`   | `null`           | absence of data

<span class="caption">Figure MF-1: Data Type Markers</span>

Marker  | Carbon Type      | Descriptions
--------|------------------|-------------------------------------------
`[{]`   | `object`         | marks begin of an object
`[}]`   | `object`         | marks end of an object
`[[]`   | `array`          | marks begin of an array
`[]]`   | `array`          | marks end of an array
`[1]`   | `u8`             | an unsigned 8-bit integer column
`[2]`   | `u16`            | an unsigned 16-bit integer column
`[3]`   | `u32`            | an unsigned 32-bit integer column
`[4]`   | `u64`            | an unsigned 64-bit integer column
`[5]`   | `u8`             | an signed 8-bit integer column
`[6]`   | `u16`            | an signed 16-bit integer column
`[7]`   | `u32`            | an signed 32-bit integer column
`[8]`   | `u64`            | an signed 64-bit integer column
`[R]`   | `float`          | a single-precision floating number column
`[B]`   | `boolean`        | a three-valued logic column



<span class="caption">Figure MF-2: Container Type Markers</span>

Marker  | Carbon Type      | Descriptions
--------|------------------|--------------------------------------------------
`[?]`   | `nokey`          | no record identifier assigned
`[*]`   | `autokey`        | record identifier is auto-generated a `u64` value
`[+]`   | `ukey`           | record identifier is user-defined a `u64` value 
`[-]`   | `ikey`           | record identifier is user-defined a `i64` value 
`[!]`   | `ikey`           | record identifier is user-defined a `string` value 

<span class="caption">Figure MF-3: Record Identifier Type Markers</span>

Marker  | Carbon Type      | Descriptions
--------|------------------|-------------------------
`[0]`   | `reserved`       | marks a single reserved byte

<span class="caption">Figure MF-4: Special Purpose Markers</span>