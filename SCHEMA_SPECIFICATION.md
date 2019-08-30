# CARBON-Schema Specification

## Introduction
We want to add the possibility to compare CARBON files to a given schema description file. This allows for users to define restrictions on how a CARBON document has to look like in order to add it to an existing document collection. In the long run this shall also apply to changes on collections.

### draft-00 (2019-08-22)

#### KEYWORDS

###### type
Type requires the data field to be of a specific type. Valid data types are:
- boolean
- u8 (unsigned 8-bit int)
- u16
- u32
- u64
- i8 (signed 8-bit int)
- i16
- i32
- i64
- float
- string
- binary
- custom binary
- null
- array
- column
- object
- record

**Example**:
```
{ "type": "string" }
```

###### required
This value should be a boolean. If set to true the associated field has to be defined and cannot be NULL.
**Example**:
```
{ "required": ["foo", "bar"] }
```

###### less / greater / lessOrEqual / greaterOrEqual
Define minimum and / or maximum for a value. The value has to be of a numeric type.
**Example**:
```
```

###### minLength / maxLength
Set a minimum or maximum length for a string or binary value. Unicode characters count as the length of one.
**Example**:
```
```    

###### format
Data has to be of a given format. Supported formats are: regex, date, time, email, uri. Format implies the data type string.
**Example**:
```
```

###### formatLess / formatGreater / formatLessOrEqual / formatGreaterOrEqual
Define a minimum and / maximum value for format types date and time.
**Example**:
```
```

###### minItems / maxItems
Set a minimum and / or maximum number of items contained in Arrays, Objects, Columns or Records.
**Example**:
```
```

###### uniqueItems
This value should be a boolean. If set to true, values in associated Array, Column or Record have to be unique within this list.

###### items
Define valid types for given Array. If the YAML sequence defines only one type, then all array elements have to be of this type. Otherwise the arrays element at position x has to be of the type defined at position x in the YAML sequence.
**Example**:
```
```

###### 
