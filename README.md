## A sample variadic iterator to iterate any arbitrary depth hierachies ##

### A use case: ###

Imagine there is a hierarchy of transient objects wi parent-child relationship. 
Each object has a name (/id) and can be identified by a composite unique name made of set of its parent names 
and its own name. The names could be of any type, for instance fixed size strings. In practice The reason of using fixed size strings vs dynamic stl strings could be dictated for example by desire to minimize dynamic memory allocations.
The objects on each level of the hierarchy can be stored in a container with that composite name used as an access key.
Relation between any parent and child is one-to-many.
For example let's say we have a hierarchy of objects `A->B->C` with A as a root parent and C is a leaf.
Whenever there is a data structure like that one there is a good chance one needs to iterate over its items.
In practice I came across a need to do a kind of batch-processing ranges of leaf objects in such hierarchy.
Under batch-processing it is meant that each leaf object in a certain range is processed by a chosen function. The range is defined by on iterator pointing to the first object (or object composite name) and number of objects.   
Iteration over leaf objects of a particular hierarchy is relatively trivial. 
However if there is a need to batch-process hierarchies of different types with various depth a lot of the code shall be duplicated. 
let's say we have `A->B->C` and `D->C->E->F` It would be nice to do processing without code duplication  
