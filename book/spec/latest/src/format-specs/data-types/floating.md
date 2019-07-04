# Floating Numbers

Carbon Type  | Description                              | Size      | *null*-Value          | Marker 
-------------|------------------------------------------|-----------|-----------------------|------------------
`float`      | single-precision floating number | 4 byte  | `null` or `NAN` | `r` 



## As Field Value


Description              | Size      | Marker          | Payload
-------------------------|-----------|-----------------|---------------
 32-bit floating number  | 1+4 bytes | `[r]` (real)    | a `float` value
 

### Example

JSON snippet
```json
[23.3, 42.0]
```

A (compacted) Carbon file, which encodes the JSON array as `array` of one `i8` and one `u8` integer.

```
[[] [r][23.3] [r][42.0] []]
```

## As Column Value


Column Type Marker               | Marker Size | Element Size | Block
---------------------------------|-------------|--------------| ---------------
 `[r]`, 32-bit floating numbers  | 1 byte      | 4 byte each  | a `float` value
 


### Example

JSON snippet
```json
[23.3, 42.0]
```

A (compacted) Carbon file, which encodes the JSON array as a `float` column.

```
[(] [r](2)(2) [23.3][42.0] [#]
```