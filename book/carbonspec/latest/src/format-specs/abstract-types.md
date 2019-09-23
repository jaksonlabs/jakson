# Abstract Types

Carbon supports abstract types, such as *lists* or *maps*. In Carbon an abstract type is of one of the following classes:

1. **Abstract Base Type**. An abstract base type does not need further treatment to guarantee their properties (*unsorted*, and *not duplicate-free*)
2. **Derived Abstract Type**. An abstract base type can be marked as a particular abstract type with further properties (such as *uniqueness* of contained elements), enabling the application to check certain promises and guarantees

Both, [array containers](container-types/arrays.md) and [column containers](container-types/columns.md) are of the abstract base type [`unsorted-multiset`](abstract-base-types.md), while an [object container](container-types/objects.md) is of the abstract base type [`unsorted-multimap`](abstract-base-types.md). 

If an applications requires more guarantees and properties to abstract types, such as uniquness of elements or duplicate-freeness, abstract base types can be annotated, effectively converting them to [derviced abstract types](derived-abstract-types.md).