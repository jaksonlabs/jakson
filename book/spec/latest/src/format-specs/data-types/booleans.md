# Booleans

## As Field Value


Description           | Size   | Marker   | Payload 
----------------------|--------|----------|---------
JSON `true` constant  | 1 byte | `[t]`    | *none*  
JSON `false` constant | 1 byte | `[f]`    | *none*  

### Example

JSON snippet
```json
[true, false]
```

A (compacted) Carbon file

```
[[] [t] [f] []]
```

## As Column Value


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

A (compacted) Carbon file, which encodes the JSON array as `boolean` column.

```
[(] [B][3][3] [0][1][2] [)]
```