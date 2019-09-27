# Container Types

Carbon Type  | Description                                 | Size | JSON Mapping 
-------------|---------------------------------------------|-----------------|------------
`array`      | list of variable-typed elements             | 2<sup>+</sup> bytes     | array (unconstrained)
`column`     | list of fixed-typed elements                | 3<sup>+</sup> bytes     | array (constrained)
`object`     | list of key-value pairs (properties)        | **TODO**     | object (unconstrained)
`record`     | identifiable, versioned `array`             | **TODO**     | JSON file / "document"

<span class="caption">Table CT-1: Container Types in Carbon</span>



## Container Type Support

### Object, Array and Record Containers

```
true, false, u8, u16, u32, u64, i8, i16, i32, i64, float, string_buffer, 
binary, custom binary, null, array, column-u8, column-u16, column-u32, 
column-u64, column-i8, column-i16, column-i32, column-i64, column-float, 
column-boolean, object 
```

<span class="caption">Table CT-2: Container Type Support for `array` and `record` Containers</span>


### Column Containers

```
boolean, u8, u16, u32, u64, i8, i16, i32, i64, float
```

<span class="caption">Table CT-3: Container Type Support for `column` Containers</span>