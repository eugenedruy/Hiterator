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

### Implementation and test cases: ###

The test case represent simple hierarchies of 
RentalCompany->Office->Car (depth of 3 levels), (let's call it Hier3) 
and
Office->Car (depth of 2 levels) (let's call it Hier2)
where -> denotes parent to child relationship. 
The test cases iterate hierarchy objects wih processing of each object being iterated.
Processing can be anything, but for test case it was chosen to be just printing   
At the end of the batch iteration a client receives a key to resume iteration for the next batch.

>#### Adapter delegation ( use of printN function) ####
A relatively trivial approach.  
It is not parameterized with a processor function, but simply calls a predefined adapter function. (Adapters<HierarchyType>::print, where HierarchyType is etier Hier3 or Hier2 described above) 
The adapter however is generic and is capable of expanding a key of arbitrary depth when delegating it to a print method of the leaf type in the hierarchy. 
Obviously the problem is lack of flexinility. The adapter is predefined and use of a different processor would require modifying the hardcoded adapter 

>#### Iterator with adapter parameterization ( use of BatchIterator::handleNWithTraitsAdapter function) ####
