# Booleans

Carbon Type  | Description             | Size                | *null*-Value      | Marker 
-------------|-------------------------|---------------------|-------------------|------------------
`true`       | the truth value *true*  | 0 byte              | `null`            | `[t]`
`false`      | the truth value *false* | 0 byte              | `null`            | `[f]`
`boolean`    | a three-valued value    | 0<sup>+</sup> bytes | *dedicated value* | *dedicated value*


## Encoding as Field Value


### Example

JSON snippet
```json
[true, false]
```

A (compacted) Carbon `array`

```
[[] [t] [f] []]
```

## Encoding as Column Value


Column Type Marker      | Marker Size | Value   | Element Size | Block   
------------------------|-------------|---------|--------------|---------
`[B]`, boolean values   | 1-byte      | `false` | 1 byte each  | `[0]`   
`[B]`, boolean values   | 1-byte      | `true`  | 1 byte each  | `[1]`   
`[B]`, boolean values   | 1-byte      | `null`  | 1 byte each  | `[2]`   


### Example

JSON snippet
```json
[false, true, null]
```

A (compacted) Carbon `column-boolean`.

```
[B](3)(3) [0][1][2]
```