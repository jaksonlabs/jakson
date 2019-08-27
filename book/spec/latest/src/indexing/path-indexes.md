# Path Indexes

- Carbon file structure is optimized for memory footprint (e.g., variable-length integers) and for embedding of variable-length data (e.g., binary or character strings)
- For path evaluation, the Carbon file structure is not optimal since it is not designed fast traversals (e.g., variable-length integers must be interpreted, variable-length fields must be skipped)
- To improve path evaluation, an optional index, the path index, may be created to improve traversal operations
- Like Carbon files, the index is a tree structure that is embedded ("flattened") into a continous memory block to increase CPU data cache efficiency.
- In contrast to Carbon files, the path index is constructed for fix-length-only data types that are more optimal for reading (e.g., no interpreation is needed). Further, for list (array and column) containers, seeking to a particular index is a constant operation. 

Key difference of path index

## Structure Overview

```
<record-ref> <paths>
```

### Record References

```
<record-ref> 
```


```
<primary-key> [commit-hash]
```

- `<primary-key>` is record identification, see Section about [Identification](../format-specs/identification.md)

- `[commit-hash]` is version of record that is indexes with this index, see Section about [Revision Control](update-spec/revision-control.md)		

### Example

JSON file
```json
[
   {
      "a":null,
      "b":[ 1, 2, 3 ],
      "c":{
         "msg":"Hello, World!"
      }
   },
   {
      "a":42,
      "b":[ ],
      "c":null
   }
]
```

Path Index
```
[?][0]
[a][](0x0)(2)(0xf)(0x34)
        [a][{](0x2)(3)(0x16)(0x19)(0x2a)
                    [P][n](0x3)
                    [P][1](0x8)(0x6)(3)(0x21)(0x24)(0x27)
                                [A][c](0xb)
                                [A][c](0xc)
                                [A][c](0xd)
                    [P][{](0x10)(0xe)(1)(0x30)
                                [P][s](0x15)(0x11)
        [a][{](0x26)(3)(0x3b)(0x3f)(0x44)
                    [P][c](0x29)(0x27)
                    [P][[](0x2d)(0x2b)(0)
                    [P][n](0x2f)
```

Path Index (formatted as Json)
```json
{
   "record-association":{
      "key-type":"nokey",
      "commit-hash":"0000000000000000"
   },
   "index":{
      "parent":"record",
      "record-reference":{
         "container":null,
         "offset":null
      },
      "nodes":{
         "element-count":2,
         "element-offsets":[
            "0xf",
            "0x34"
         ],
         "elements":[
            {
               "parent":"array",
               "record-reference":{
                  "container":"object",
                  "offset":"0x2"
               },
               "nodes":{
                  "element-count":3,
                  "element-offsets":[
                     "0x16",
                     "0x19",
                     "0x2a"
                  ],
                  "elements":[
                     {
                        "type":"key",
                        "record-reference":{
                           "container":"null",
                           "offset":null
                        },
                        "key":"0x3"
                     },
                     {
                        "type":"key",
                        "record-reference":{
                           "container":"column-u8",
                           "offset":"0x8"
                        },
                        "key":"0x6",
                        "element-count":3,
                        "element-offsets":[
                           "0x21",
                           "0x24",
                           "0x27"
                        ],
                        "elements":[
                           {
                              "type":"column",
                              "record-reference":{
                                 "container":"number-u8",
                                 "offset":"0xb"
                              }
                           },
                           {
                              "type":"column",
                              "record-reference":{
                                 "container":"number-u8",
                                 "offset":"0xc"
                              }
                           },
                           {
                              "type":"column",
                              "record-reference":{
                                 "container":"number-u8",
                                 "offset":"0xd"
                              }
                           }
                        ]
                     },
                     {
                        "type":"key",
                        "record-reference":{
                           "container":"object",
                           "offset":"0x10"
                        },
                        "key":"0xe",
                        "element-count":1,
                        "element-offsets":[
                           "0x30"
                        ],
                        "elements":[
                           {
                              "type":"key",
                              "record-reference":{
                                 "container":"string",
                                 "offset":"0x15"
                              },
                              "key":"0x11"
                           }
                        ]
                     }
                  ]
               }
            },
            {
               "parent":"array",
               "record-reference":{
                  "container":"object",
                  "offset":"0x26"
               },
               "nodes":{
                  "element-count":3,
                  "element-offsets":[
                     "0x3b",
                     "0x3f",
                     "0x44"
                  ],
                  "elements":[
                     {
                        "type":"key",
                        "record-reference":{
                           "container":"number-u8",
                           "offset":"0x29"
                        },
                        "key":"0x27"
                     },
                     {
                        "type":"key",
                        "record-reference":{
                           "container":"array",
                           "offset":"0x2d"
                        },
                        "key":"0x2b",
                        "element-count":0,
                        "element-offsets":[

                        ],
                        "elements":[

                        ]
                     },
                     {
                        "type":"key",
                        "record-reference":{
                           "container":"null",
                           "offset":null
                        },
                        "key":"0x2f"
                     }
                  ]
               }
            }
         ]
      }
   }
}
```