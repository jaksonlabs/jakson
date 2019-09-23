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
`[1]`   | `column-u8`             | an unsigned 8-bit integer column
`[2]`   | `column-u16`            | an unsigned 16-bit integer column
`[3]`   | `column-u32`            | an unsigned 32-bit integer column
`[4]`   | `column-u64`            | an unsigned 64-bit integer column
`[5]`   | `column-i8`             | an signed 8-bit integer column
`[6]`   | `column-i16`            | an signed 16-bit integer column
`[7]`   | `column-i32`            | an signed 32-bit integer column
`[8]`   | `column-i64`            | an signed 64-bit integer column
`[R]`   | `column-float`          | a single-precision floating number column
`[B]`   | `column-boolean`        | a three-valued logic column

<span class="caption">Figure MF-2: Container Type Markers</span>


Begin Marker | Abstract Type        | Base | Container         | Element | Distinct | Sorted
-------------|----------------------|------|-------------------|---------|----------|--------
`[{]`        | `unsorted-multimap`  | yes  | `object`          | pair    | no       | no
`[~]`        | `sorted-multimap`    | no   | `object`          | pair    | no       | yes
`[:]`        | `unsorted-map`       | no   | `object`          | pair    | yes      | no
`[#]`        | `sorted-map`         | no   | `object`          | pair    | yes      | yes



<span class="caption">Figure MF-2: Abstract Types on `object` containers</span>

Begin Marker | Abstract Type        | Base | Container         | Element | Distinct | Sorted
-------------|----------------------|------|-------------------|---------|----------|--------
`[[]`        | `unsorted-multiset`      | yes  | `array`           | value   | no       | no
`[<]`        | `sorted-multiset`        | no   | `array`           | value   | no       | yes 
`[/]`        | `unsorted-set`       | no   | `array`           | value   | yes      | no 
`[=]`        | `sorted-set`         | no   | `array`           | value   | yes      | yes 


<span class="caption">Figure MF-2: Abstract Types on `array` containers</span>

Begin Marker | Abstract Type        | Base | Container         | Element | Distinct | Sorted
-------------|----------------------|------|-------------------|---------|----------|--------
`[1]`        | `unsorted-multiset`  | yes  | `column-u8`       | value   | no       | no
`[SOH]`      | `sorted-multiset`    | no   | `column-u8`       | value   | no       | yes 
`[STX]`      | `unsorted-set`       | no   | `column-u8`       | value   | yes      | no 
`[ETX]`      | `sorted-set`         | no   | `column-u8`       | value   | yes      | yes 
`[2]`        | `unsorted-multiset`  | yes  | `column-u16`      | value   | no       | no
`[ENQ]`      | `sorted-multiset`    | no   | `column-u16`      | value   | no       | yes 
`[ACK]`      | `unsorted-set`       | no   | `column-u16`      | value   | yes      | no 
`[BEL]`      | `sorted-set`         | no   | `column-u16`      | value   | yes      | yes 
`[3]`        | `unsorted-multiset`  | yes  | `column-u32`      | value   | no       | no
`[TAB]`      | `sorted-multiset`    | no   | `column-u32`      | value   | no       | yes 
`[LF]`       | `unsorted-set`       | no   | `column-u32`      | value   | yes      | no 
`[VT]`       | `sorted-set`         | no   | `column-u32`      | value   | yes      | yes 
`[4]`        | `unsorted-multiset`  | yes  | `column-u64`      | value   | no       | no
`[CR]`       | `sorted-multiset`    | no   | `column-u64`      | value   | no       | yes 
`[S0]`       | `unsorted-set`       | no   | `column-u64`      | value   | yes      | no 
`[S1]`       | `sorted-set`         | no   | `column-u64`      | value   | yes      | yes 
`[5]`        | `unsorted-multiset`  | yes  | `column-i8`       | value   | no       | no
`[DC1]`      | `sorted-multiset`    | no   | `column-i8`       | value   | no       | yes 
`[DC2]`      | `unsorted-set`       | no   | `column-i8`       | value   | yes      | no 
`[DC3]`      | `sorted-set`         | no   | `column-i8`       | value   | yes      | yes 
`[6]`        | `unsorted-multiset`  | yes  | `column-i16`      | value   | no       | no
`[NAK]`      | `sorted-multiset`    | no   | `column-i16`      | value   | no       | yes 
`[SYN]`      | `unsorted-set`       | no   | `column-i16`      | value   | yes      | no 
`[ETB]`      | `sorted-set`         | no   | `column-i16`      | value   | yes      | yes 
`[7]`        | `unsorted-multiset`  | yes  | `column-i32`      | value   | no       | no
`[EM]`       | `sorted-multiset`    | no   | `column-i32`      | value   | no       | yes 
`[SUB]`      | `unsorted-set`       | no   | `column-i32`      | value   | yes      | no 
`[ESC]`      | `sorted-set`         | no   | `column-i32`      | value   | yes      | yes 
`[8]`        | `unsorted-multiset`  | yes  | `column-i64`      | value   | no       | no
`[GS]`       | `sorted-multiset`    | no   | `column-i64`      | value   | no       | yes 
`[RS]`       | `unsorted-set`       | no   | `column-i64`      | value   | yes      | no 
`[US]`       | `sorted-set`         | no   | `column-i64`      | value   | yes      | yes 
`[R]`        | `unsorted-multiset`  | yes  | `column-float`    | value   | no       | no
`["]`        | `sorted-multiset`    | no   | `column-float`    | value   | no       | yes 
`[$]`        | `unsorted-set`       | no   | `column-float`    | value   | yes      | no 
`[.]`        | `sorted-set`         | no   | `column-float`    | value   | yes      | yes 
`[B]`        | `unsorted-multiset`  | yes  | `column-boolean`  | value   | no       | no
`[_]`        | `sorted-multiset`    | no   | `column-boolean`  | value   | no       | yes 
`[']`        | `unsorted-set`       | no   | `column-boolean`  | value   | yes      | no 
`[DEL]`      | `sorted-set`         | no   | `column-boolean`  | value   | yes      | yes 






<span class="caption">Figure MF-2: Abstract Types on `column` containers</span>


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


```
[NUL], [EOT], [BS], [FF], [DLE], [DC4], [CAN], [FS], [^], [\], [@] 
```

<span class="caption">Figure MF-4: Unused but reserved Markers</span>