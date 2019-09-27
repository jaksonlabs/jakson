# Character Strings

Carbon Type     | Description                                         | Size      | *null*-Value | Marker 
----------------|-----------------------------------------------------|-----------|--------------|--------
`string_buffer`        | a (Pascal) character string_buffer of `n` characters | `n` bytes | `null`       | `[s]`


## Encoding as Field Value

```
[s](n) <character-string_buffer>
```


Description                             | Size          | Marker          | Payload
----------------------------------------|---------------|-----------------|-------------------------------------------------
 a `n`-char string_buffer  | 1 + `l` + `n` bytes | `[s]` (string_buffer)  | `(n)` length (`l` byte), `n` chars
 

### Example

JSON snippet
```json
["Hello", "World", "!"]
```

A (compacted) Carbon file, which encodes the JSON array as `array` of character-string_buffer values.

```
[[] [s](5)[Hello] [s](5)[World] [s](1)[!] []]
```

## Encoding as Column Value


Character strings are not supported for column containers.
