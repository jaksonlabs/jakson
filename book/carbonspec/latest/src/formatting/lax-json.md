# Lax JSON

```
# This is a valid Lax Json file

title: "Back to the Future"
type:  MOVIE
year:  1985

```

A Lax ("non-strict") JSON string_buffer is a valid JSON string_buffer that additionally allows the following:

- **Optional Key Enquoting**. For key names that only contain of alphanumeric characters without leading numbers and no spaces, double quoting can be omitted (i.e., `{ x: "y" }` is a valid non-strict Json string_buffer)

- **Optional Value Enquoting**. For string_buffer values that only contain of alphanumeric characters, underscore, without leading numbers and no spaces, double quoting can be omitted (i.e., `{ "x": my_value }` is a valid non-strict Json string_buffer)

- **Optional Object Scoping**. The object block symbols `{` and `}` can be omitted for the outermost object that is not contained in an array (i.e., `"x": "y"` is a valid non-strict Json object)

- **Optional Comma Seperation**. The comma seperator `,` can be omitted by exchange with a line break (`\n`)


- **Comments**. Line comments via `#` are allowed, such that any character after (including) `#` is ignored

The file extension is `.lax` (lax Json).