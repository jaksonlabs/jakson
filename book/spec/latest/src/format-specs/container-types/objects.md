# Objects

Carbon Type    | Description                          | Size                | *null*-Value | Marker
---------------|--------------------------------------|---------------------|--------------|------------
`object`       | list of key-value pairs (properties) | **TODO** bytes | `null` | `{`, `}`

## Encoding

```
[{] <props>... [}]
```

### Properties

```
<props>
```

```
<key-string> <value>
```



#### Keys

```
<key-string>
```

A `<key-string>` is any value of type `string` where the `[s]` marker is omitted.

#### Values

```
<value>
```

A `<value>` is any value of a supported type.

```
true, false, u8, u16, u32, u64, i8, i16, i32, i64, float, string, 
binary, custom binary, null, array, column, object 
```


### Capacities

```
...
```

### Example


JSON snippet
```json
{
   "title":"Back to the Future",
   "sub-title":null,
   "year": 1985,
   "imdb-rating": 8.5,
   "keywords":[
      "time travel",
      "deloren",
      "comedy"
   ],
   "release-dates": [
   	  1985, 1986, 1987, 
   	  1992, 2008, 2010, 
   	  2012, 2015, 2016
   ]
}
```

A (compacted) Carbon `object`.

```
[{]
   (5)[title] [s](18)[Back to the Future]
   (9)[sub-title] [n]
   (4)[year] [c][1985]
   (11)[imdb-rating] [r][8.5]
   (8)[keywords] [[] 
      [s](11)[time travel] 
      [s](7)[deloren]       
      [s](6)[comedy]            
   []]
   (13)[release-dates] [1](9)(9)
      [1985] [1986] [1987]
      [1992] [2008] [2010]
      [2012] [2015] [2016]
[}]   
```
