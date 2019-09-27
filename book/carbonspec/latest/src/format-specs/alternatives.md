# Alternatives

## Running Example

JSON snippet
```json
{
   "title":"Back to the Future",
   "sub-title":null,
   "year":1985,
   "imdb-rating":8.5,
   "keywords":[
      "time travel",
      "delorean",
      "comedy"
   ],
   "release-dates":[
      1985, 1986, 1987,
      1992, 2008, 2010,
      2012, 2015, 2016
   ]
}
```


Carbon `record` for JSON snippet

```
[nokey, 1B]
[array-begin, 1B]
[object-begin, 1B]
   (str-len, 1B)[title, 5B] [string_buffer-field, 1B](str-len, 1B)[Back to the Future, 18B]
   (str-len, 1B)[sub-title, 9B] [null-field, 1B]
   (str-len, 1B)[year, 4B] [short-integer-field, 1B][1985, 2B]
   (str-len, 1B)[imdb-rating, 11B] [float-field, 1B][8.5, 4B]
   (str-len, 1B)[keywords, 8B] 
      [array-begin, 1B] 
         [string_buffer-field, 1B](str-len, 1B)[time travel, 11B] 
         [string_buffer-field, 1B](str-len, 1B)[delorean, 8B]       
         [string_buffer-field, 1B](str-len, 1B)[comedy, 6B]            
      [array-end, 1B]
   (str-len, 1B)[release-dates,13B] 
      [short-int-column, 1B](num-elems, 1B)(cap-elems, 1B)
         [1985, 2B] 
         [1986, 2B] 
         [1987, 2B]
         [1992, 2B] 
         [2008, 2B] 
         [2010, 2B]
         [2012, 2B]
         [2015, 2B]
         [2016, 2B]
[object-end, 1B] 
[array-end, 1B] 
```

144 byte

Format                         | Document Size
-------------------------------|--------------
Plain-Text JSON                | 187 byte
Binary JSON (BSON)             | 219 byte
Universal Binary JSON (UBJSON) | 155 byte
Columnar Binary JSON (Carbon)  | 144 byte