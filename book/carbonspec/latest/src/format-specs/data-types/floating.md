# Floating Numbers

Carbon Type  | Description                              | Size      | *null*-Value          | Marker 
-------------|------------------------------------------|-----------|-----------------------|------------------
`float`      | single-precision floating number | 4 byte  | `null` or `NAN` | `r` 



## Encoding as Field Value


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

## Encoding as Column Value


Column Type Marker               | Element Size | Block
---------------------------------|--------------| ---------------
 `[R]`, 32-bit floating numbers  | 4 byte each  | a `float` value
 


### Example

JSON snippet
```json
[23.3, 42.0]
```

A (compacted) Carbon `column-float`.

```
[R] (2)(2) [23.3][42.0]
```