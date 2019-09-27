# Nulls

Carbon Type | Description     | Size   | *null*-Value | Marker 
------------|-----------------|--------|--------------|------------------------
`null`      | absence of data (user-defined) | 0<sup>+</sup> byte | `null` value | `[n]` or dedicated value


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

> There is no dedicated column type for `null`-only lists. Instead use `array` types that contain only `null` values. 

The occurence of a `null` value in a column is done by encoding the absence of data with a dedicated value taken from the column type domain. Note that this dedicated value is reserved for this use, and is not in the value domain of the corresponding Carbon type.


Data Type                | Column Marker | Null Size | Null Constant                 |
-------------------------|---------------|-----------|-------------------------------|
booleans values          | `[B]`         | 1 byte    | `2`                           |
unsigned 8-bit int.      | `[1]`         | 1 byte    | `255`                         |
unsigned 16-bit int.     | `[2]`         | 2 bytes   | `65,535`                      |
unsigned 32-bit int.     | `[3]`         | 4 bytes   | `4,294,967,295`               |
unsigned 64-bit int.     | `[4]`         | 8 bytes   | `18,446,744,073,709,551,615`  |
signed 8-bit integers    | `[5]`         | 1 byte    | `−128`                        |
signed 16-bit int.       | `[6]`         | 2 bytes   | `−32,768`                     |
signed 32-bit int.       | `[7]`         | 4 bytes   | `−2,147,483,648`              |
signed 64-bit int.       | `[8]`         | 8 bytes   | `−9,223,372,036,854,775,808`  |
32-bit floats            | `[R]`         | 4 bytes   | `NAN`                         |

### Example

JSON snippet
```json
[42, 23, null]
```

A (compacted) Carbon `column-u8`

```
[1](3)(3) [42][23][255]
```