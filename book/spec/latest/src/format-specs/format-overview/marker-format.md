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
`[{]`        | `unsorted_multimap`  | yes  | `object`          | pair    | no       | no
`[~]`        | `sorted_multimap`     | no   | `object`          | pair    | no       | yes
`[:]`        | `unsorted_map`       | no   | `object`          | pair    | yes      | no
`[#]`        | `sorted_map`         | no   | `object`          | pair    | yes      | yes



<span class="caption">Figure MF-2: Abstract Types on `object` containers</span>

Begin Marker | Abstract Type        | Base | Container         | Element | Distinct | Sorted
-------------|----------------------|------|-------------------|---------|----------|--------
`[[]`        | `unsorted_list`      | yes  | `array`           | value   | no       | no
`[<]`        | `sorted_list`        | no   | `array`           | value   | no       | yes 
`[/]`        | `unsorted_set`       | no   | `array`           | value   | yes      | no 
`[=]`        | `sorted_set`         | no   | `array`           | value   | yes      | yes 


<span class="caption">Figure MF-2: Abstract Types on `array` containers</span>

Begin Marker | Abstract Type        | Base | Container         | Element | Distinct | Sorted
-------------|----------------------|------|-------------------|---------|----------|--------
`[1]`        | `unsorted_list`      | yes  | `column-u8`       | value   | no       | no
`[SOH]`        | `sorted_list`        | no   | `column-u8`       | value   | no       | yes 
`[STX]`        | `unsorted_set`       | no   | `column-u8`       | value   | yes      | no 
`[ETX]`        | `sorted_set`         | no   | `column-u8`       | value   | yes      | yes 
`[2]`        | `unsorted_list`      | yes  | `column-u16`       | value   | no       | no
`[ENQ]`        | `sorted_list`        | no   | `column-u16`       | value   | no       | yes 
`[ACK]`        | `unsorted_set`       | no   | `column-u16`       | value   | yes      | no 
`[BEL]`        | `sorted_set`         | no   | `column-u16`       | value   | yes      | yes 
`[3]`        | `unsorted_list`      | yes  | `column-u32`       | value   | no       | no
`[TAB]`        | `sorted_list`        | no   | `column-u32`       | value   | no       | yes 
`[LF]`        | `unsorted_set`       | no   | `column-u32`       | value   | yes      | no 
`[VT]`        | `sorted_set`         | no   | `column-u32`       | value   | yes      | yes 
`[4]`        | `unsorted_list`      | yes  | `column-u64`       | value   | no       | no
`[CR]`        | `sorted_list`        | no   | `column-u64`       | value   | no       | yes 
`[S0]`        | `unsorted_set`       | no   | `column-u64`       | value   | yes      | no 
`[S1]`        | `sorted_set`         | no   | `column-u64`       | value   | yes      | yes 
`[5]`        | `unsorted_list`      | yes  | `column-i8`       | value   | no       | no
`[DC1]`        | `sorted_list`        | no   | `column-i8`       | value   | no       | yes 
`[DC2]`        | `unsorted_set`       | no   | `column-i8`       | value   | yes      | no 
`[DC3]`        | `sorted_set`         | no   | `column-i8`       | value   | yes      | yes 
`[6]`        | `unsorted_list`      | yes  | `column-i16`       | value   | no       | no
`[NAK]`        | `sorted_list`        | no   | `column-i16`       | value   | no       | yes 
`[SYN]`        | `unsorted_set`       | no   | `column-i16`       | value   | yes      | no 
`[ETB]`        | `sorted_set`         | no   | `column-i16`       | value   | yes      | yes 
`[7]`        | `unsorted_list`      | yes  | `column-i32`       | value   | no       | no
`[EM]`        | `sorted_list`        | no   | `column-i32`       | value   | no       | yes 
`[SUB]`        | `unsorted_set`       | no   | `column-i32`       | value   | yes      | no 
`[ESC]`        | `sorted_set`         | no   | `column-i32`       | value   | yes      | yes 
`[8]`        | `unsorted_list`      | yes  | `column-i64`       | value   | no       | no
`[GS]`        | `sorted_list`        | no   | `column-i64`       | value   | no       | yes 
`[RS]`        | `unsorted_set`       | no   | `column-i64`       | value   | yes      | no 
`[US]`        | `sorted_set`         | no   | `column-i64`       | value   | yes      | yes 
`[R]`        | `unsorted_list`      | yes  | `column-float`       | value   | no       | no
`["]`        | `sorted_list`        | no   | `column-float`       | value   | no       | yes 
`[$]`        | `unsorted_set`       | no   | `column-float`       | value   | yes      | no 
`[.]`        | `sorted_set`         | no   | `column-float`       | value   | yes      | yes 
`[B]`        | `unsorted_list`      | yes  | `column-boolean`       | value   | no       | no
`[_]`        | `sorted_list`        | no   | `column-boolean`       | value   | no       | yes 
`[']`        | `unsorted_set`       | no   | `column-boolean`       | value   | yes      | no 
`[DEL]`        | `sorted_set`         | no   | `column-boolean`       | value   | yes      | yes 






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