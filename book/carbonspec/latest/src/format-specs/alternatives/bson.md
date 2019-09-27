# Binary JSON

## BSON Running Example

```
[object-size, 4B]
	[string_buffer-field, 1B] [title\0, 6B] [byte-num, 4B] [Back to the Future\0, 19B]
	[null-field, 1B] [sub-title\0, 10B] 
	[integer-field, 1B] [year\0, 5B] [1985, 4B]
	[float-field, 1B] [imdb-rating\0, 12B] [8.5, 8B]
	[array-field, 1B] [keywords\0, 9B] 
		[doc-size, 4 B]
			[string_buffer-field, 1B] [0\0, 2B] [byte-num, 4B] [time travel\0, 12B]
			[string_buffer-field, 1B] [1\0, 2B] [byte-num, 4B] [delorean\0, 9B]
			[string_buffer-field, 1B] [2\0, 2B] [byte-num, 4B] [comedy\0, 7B]
		[null, 1 B]
	[array-field, 1B] [release-dates\0, 9B] 
		[doc-size, 4 B]
			[integer-field, 1B] [0\0, 2B] [1985, 4B]
			[integer-field, 1B] [1\0, 2B] [1986, 4B]
			[integer-field, 1B] [2\0, 2B] [1987, 4B]
			[integer-field, 1B] [3\0, 2B] [1992, 4B]
			[integer-field, 1B] [4\0, 2B] [2008, 4B]
			[integer-field, 1B] [5\0, 2B] [2010, 4B]
			[integer-field, 1B] [6\0, 2B] [2012, 4B]
			[integer-field, 1B] [7\0, 2B] [2015, 4B]
			[integer-field, 1B] [8\0, 2B] [2016, 4B]
		[null, 1 B]
	[null, 1B]
```

219 byte


## Comparison to Carbon

### Design Goals

#### Commonalities

#### Differences

### Encoding

#### Commonalities

#### Differences
