# Nil

A `nil` is used to express values that do not exists, or which have no valid path. The value `nil` occurs whenever a carbon `record`
is queried for a non-existing path using a [dot path expression](traversals-queries/path-eval/path-eval.md).

Carbon Type | Description     | Size   | *null*-Value | Marker 
------------|-----------------|--------|--------------|------------------------
`nil`      | absence of data (structure-defined) | 0 | *none* | `[N]`


## Encoding as Field Value


Carbon File snippet

```
[[] [N] []] 
```

Note that `nil` does not match to a Json `null`, use a Carbon `null` type instead. For more, see Section [Nulls](format-specs/data-types/nulls.md).

Since there is no mapping from a Carbon `nil` value to a Json value for Json-formatted output, a `nil` value is formatted as a Json-string_buffer prefixed with `_`.

Json File snippet

```json
"_nil"
```

Note that this is a uni-directed mapping from Carbon files to Json files. The opposite direction, i.e., mapping from `"_nil"` to `nil`, is not defined.


## Encoding as Column Value

`nil` values are not supported for column containers.