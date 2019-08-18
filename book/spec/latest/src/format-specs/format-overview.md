# Format Overview


```
<primary-key> [commit-hash] [[] <values>... []]...
```

<span class="caption">Figure FO-1: Carbon File Record Format</span>

A **marker** `x` has always a size of 1 byte. 

## Document Identification

```
<primary-key>
```

See more about identity of Carbon records in the Section about [Identification](../format-specs/identification.md).

## Revision Managment

```
[commit-hash]
```

> The `commit-hash` field for a carbon `record` which has no primary key (i.e., `nokey` is set), is omitted. In any case of revison, the commit hash of a `record` without primary key is `0`. See more in section about record [identification](identification.md).

The `commit-hash` field requires 64bit.

The initial commit hash for an new `record` having some primary key type, is a (almost global) unique randomly generated number. 

## Values

```
<values>
```

## Capacitities

```
...
```

## Examples

### Example (Array)


JSON snippet
```json
[ "The", "Number", 23 ]
```

A (compacted) Carbon `record` with user-defined primary key `21` in revision `5` encoding the JSON array as `array`.

```
[ukey][21](5) [[] [s](3)[The] [s](6)[Number] [c][23] []] 
```


### Example (Column)


JSON snippet
```json
[ 23, 24, 25 ]
```

A (compacted) Carbon `record` with user-defined primary key `21` in revision `5` encoding the JSON array as `column-u8` integer column with `3` elements and a capacity of `3`.

```
[ukey][21](5) [1](3)(3) [23][24][25] []] 
```
