# Nulls

Carbon Type | Description     | Size   | *null*-Value | Marker 
------------|-----------------|--------|--------------|------------------------
`null`      | absence of data | 0<sup>+</sup> byte | `null` value | `[n]` or dedicated value


## Encoding as Field Value


Description           | Size   | Marker  | Payload |
----------------------|--------|---------|---------|
JSON `null` constant  | 1 byte | `[n]`   | *none*  |

### Example

JSON snippet
```json
[ null ]
```

Carbon File snippet

```
[[] [n] []] 
```

## Encoding as Column Value


Column Type Marker              | Marker Size | Element Size  | Dedicated Value               |
--------------------------------|-------------|---------------|-------------------------------|
`[n]`, null values              | 1 byte      | 1 byte each   | `n`                           |
`[B]`, booleans values          | 1 byte      | 1 byte each   | `2`                           |
`[c]`, unsigned 8-bit int.      | 1 byte      | 1 byte each   | `255`                         |
`[d]`, unsigned 16-bit int.     | 1 byte      | 2 bytes each  | `65,535`                      |
`[i]`, unsigned 32-bit int.     | 1 byte      | 4 bytes each  | `4,294,967,295`               |
`[l]`, unsigned 64-bit int.     | 1 byte      | 8 bytes each  | `18,446,744,073,709,551,615`  |
`[C]`, signed 8-bit integers    | 1 byte      | 1 byte each   | `−128`                        |
`[D]`, signed 16-bit int.       | 1 byte      | 2 bytes each  | `−32,768`                     |
`[I]`, signed 32-bit int.       | 1 byte      | 4 bytes each  | `−2,147,483,648`              |
`[L]`, signed 64-bit int.       | 1 byte      | 8 bytes each  | `−9,223,372,036,854,775,808`  |
`[r]`, 32-bit floats            | 1 byte      | 4 bytes each  | `NAN`                         |

### Example

JSON snippet
```json
[42, 23, null]
```

Carbon File snippet (JSON array encoded as `u8` column)

```
[(] [c](3)(3) [42][23][255] [#]
```