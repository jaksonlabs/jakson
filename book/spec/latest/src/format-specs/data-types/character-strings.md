# Character Strings

Type     | Description                       | Size    
---------|-----------------------------------|-------------------------
`string` | a (Pascal) ASCII character string | 1 bytes per character 


## As Field Value

```
[s](n) <character-string>
```


Description                             | Size          | Marker          | Payload
----------------------------------------|---------------|-----------------|-------------------------------------------------
 a `n`-char string  | 1 + `l` + `n` bytes | `[s]` (string)  | `(n)` length (`l` byte), `n` chars
 

### Example

JSON snippet
```json
["Hello", "World", "!"]
```

A (compacted) Carbon file, which encodes the JSON array as `array` of character-string values.

```
[[] [s](5)[Hello] [s](5)[World] [s](1)[!] []]
```

## As Column Value


Character strings are not supported for column containers.
