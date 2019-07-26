# Format Overview


```
<primary-key> (rev-number) [[] <values>... []]...
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
(rev-number)
```

> The `rev-number` field for a carbon `record` which has no primary key (i.e., `nokey` is set), is omitted. In any case of revison, the revision number of a `record` without primary key is `0`. See more in section about record [identification](identification.md).

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

A (compacted) Carbon file with object id `21` in revision `5` encoding the JSON array as `array`.

```
[21](5) [[] [s](3)[The] [s](6)[Number] [c][23] []] 
```


### Example (Column)


JSON snippet
```json
[ 23, 24, 25 ]
```

A (compacted) Carbon file with object id `21` in revision `5` encoding the JSON array as `u8` integer column.

```
[21](5) [[] [1](3)(3) [23][24][25] []] 
```
