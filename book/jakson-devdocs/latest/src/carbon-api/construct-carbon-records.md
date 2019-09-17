# Construct Carbon Records

Carbon records are implemented by the `jak_carbon` structured defined in `src/jak_carbon.h`. 

In a nutshell, Carbon records are a memory file (`jak_memfile`), the memory block (`jak_memblock`) to which this memory file writes and reads, an error and status structure (`jak_error`), and some internal variables for book-keeping. A Carbon record is a flat object that is encoded according to the [Columnar Binary JSON (Carbon) specification](http://www.carbonspec.org) by using functionality of the contained memory file to read and write safly in one continuous memory area managned by the memory block.  

In its core, a Carbon record is a set of nested vector-like structures `<data> <reserved>` where one memory area slice contains some data (`<data>`) which is typically followed by a (potentially empty) sequence of zero-valued bytes making up capacities (`<reserved>`). These capacities are used to hold some (unused) bytes in reserve in order to avoid to-often reallocation when constructing or modifying records. Especially, the entire Carbon record itself follows this approach (i.e., a Carbon file looks like `<carbon-record> <reserved>`), where the reserved memory beyond the `<carbon-record>` is called the *tail buffer*. The tail buffer is used as every other reserved piece of memory, namely to buffer growth of data, but in addition is also used as temporary (already allocated) buffer in the heap. If possible, the tail buffer is used as intermediate space for re-writing purposes. A client typically does neither notice these reserved memories (with exception of the binary size), nor access these buffers directly. However, this reserved memory is target of several optimization options, as later shown in [Section 8 (Record Optimization)](record-optimization.md).

> *Note*: A Carbon record structure `jak_carbon` can live on both the stack and the heap, but the underlying memory block, which is used to encode the actual record, always lives on the heap.

Modification on Carbon records (including their creation) are atomic operations that may consists of several sub operations, such as insertion or a particular property. Each atomic modification is managed by a record-local revision context (`jak_carbon_revise`), which is wrapped by a special context (`jak_carbon_new`) for creation of new Carbon records.

The Carbon API provides three ways for construction of new Carbon records:

1. [Parsing a JSON plain-text into a new Carbon record](construct-carbon-records/from-json.md)
2. [Manual construction of a Carbon record by API functions](construct-carbon-records/manual-construction.md)

In addition, the third way is [reconstruction of a Carbon record by a byte stream](construct-carbon-records/from-byte-streams.md).

For the first two ways, a [record identification type](record-identification.md), and an [record optimization flag](record-optimization.md) must be specified. 

The [record identification type](record-identification.md) determines what kind of primary key is used for the new record. For ease of understanding, all records that are created in the following pages will have no primary key at all (controlled by the key type `JAK_CARBON_KEY_NOKEY`). What this means and how to use alternatives is covered in the [Section about Record Identification](record-identification.md). 

The [record optimization flag](record-optimization.md) controls what to do with reserved memory for nested vector-like structures `<data> <reserved>` and the tail buffer `<reserved>` beyond the carbon record `<carbon-record> <reserved>`. For ease of understanding, all records in the following pages will be left unoptimized (controlled by the flag `JAK_CARBON_KEEP`). More details on alternatives and when to use these, is described in the [Section about Record Optimization](record-optimization.md)


The next pages give a tutorial on each construction way. 

Let's start with parsing JSON files into Carbon records.