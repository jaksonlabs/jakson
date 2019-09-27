# Records

Carbon Type    | Description                          | Size                | *null*-Value | Marker
---------------|--------------------------------------|---------------------|--------------|------------
`record`       | (potentially) identifable, (potentially) versioned `array`       | **TODO** bytes | *none* | *none*


## Example

```
[nokey] [[] ... []] 
```

## Empty Records

Json string_buffer input
```json
{ }
```

Resulting `record`

```
[nokey] [[] []] 
```

An empty `record` is a (potentially) identifable, (potentially) versioned and empty `array`. However, for compatbility with Json input semantics,
an empty `record` is treated like an empty object. Since a Json input string_buffer `[ ]` that describes an empty array, results also in an empty `record`,
one must keep this special treatment in mind.


