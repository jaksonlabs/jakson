# Undef

A `undef` is used to express values that do not exists, or which have no valid path. The value `undef` occurs whenever a carbon `record`
is queried for a non-existing path using a [dot path expression](traversals-queries/path-eval/path-eval.md).

Carbon Type | Description     | Size   | *null*-Value | Marker 
------------|-----------------|--------|--------------|------------------------
`undef`      | absence of data (structure-defined) | 0 | *none* | `[N]`


## Encoding as Field Value


Carbon File snippet

```
[[] [N] []] 
```

Note that `undef` does not match to a Json `null`, use a Carbon `null` type instead. For more, see Section [Nulls](format-specs/data-types/nulls.md).

Since there is no mapping from a Carbon `undef` value to a Json value for Json-formatted output, a `undef` value is formatted as a Json-string prefixed with `_`.

Json File snippet

```json
"_undefined"
```

Note that this is a uni-directed mapping from Carbon files to Json files. The opposite direction, i.e., mapping from `"_undefined"` to `undef`, is not defined.


## Encoding as Column Value

`undef` values are not supported for column containers.