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
      "deloren",
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
[object-begin, 1B]
   (str-len, 1B)[title, 5B] [string-field, 1B](str-len, 1B)[Back to the Future, 18B]
   (str-len, 1B)[sub-title, 9B] [null-field, 1B]
   (str-len, 1B)[year, 4B] [small-integer-field, 1B][1985, 1B]
   (str-len, 1B)[imdb-rating, 11B] [float-field, 1B][8.5, 4B]
   (str-len, 1B)[keywords, 8B] 
      [array-begin, 1B] 
         [string-field, 1B](str-len, 1B)[time travel, 11B] 
         [string-field, 1B](str-len, 1B)[deloren, 7B]       
         [string-field, 1B](str-len, 1B)[comedy, 6B]            
      [array-end, 1B]
   (str-len, 1B)[release-dates,13B] 
      [small-int-column, 1B](num-elems, 1B)(cap-elems, 1B)
         [1985, 1B] 
         [1986, 1B] 
         [1987, 1B]
         [1992, 1B] 
         [2008, 1B] 
         [2010, 1B]
         [2012, 1B]
         [2015, 1B]
         [2016, 1B]
[object-end, 1B]  
```

131 byte

Format                         | Document Size
-------------------------------|--------------
Plain-Text JSON                | 187 byte
Binary JSON (BSON)             | 218 byte
Universal Binary JSON (UBJSON) | 144 byte
Columnar Binary JSON (Carbon)  | 131 byte