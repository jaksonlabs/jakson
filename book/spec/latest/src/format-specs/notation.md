# Notation

| Notation  | Description                                       | Example                                                         |
|-----------|---------------------------------------------------|-----------------------------------------------------------------|
| `[x]`     | Fixed-length block `x` of certain number of bytes | `[{]` an object start marker, `[n]` a null value, `[21]` an integer value, `[Hello, World!]` a character string |
| `(x)`     | Varialbe-length value `x` with varying bytes | `(len)` variable-length unsigned integer encoding a length `len` |
| `<x>`     | Optional, context-dependent value `x` | `[[] <data> []]` an array block `[` `]` with content `data` that may be of varying length |
| `...`     | Memory range of zero or more zeros    | `[...]...` an empty array block containing reserved memory, followed by (unused) reserved memory |


`[x]`, `(x)`, and `<x>` are called **blocks** with content `x`.