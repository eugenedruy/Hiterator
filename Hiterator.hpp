/*
 * Hiterator.hpp
 *
 */

#ifndef HITERATOR_HPP_
#define HITERATOR_HPP_

#include <map>
#include <array>
#include <tuple>
#include <algorithm>
#include <random>



// The traits class for handling generic hierarchy objects hierarchy of variable depth
// All types of objects in a hierarchy can be different
// The depth is the number of objects in a hierarchy.
// Example: for hierarchy A->B->C the depth is 3, A is the root parent, C - is the leaf
// where A, B, C are types . Each parent has one to many relationship to its children.
// For the example above An object of type A can contain multiple objects of type B and consequently
// an object of type B can contain multiple objects of type C
// Objects on each level of the hierarchy is stored in a container
// with a composite access key made of the chain of ids (or names) of all its parents and object itsef
// So for the example C key is a composite entity made of its parents A and B ids and an id of C itself
// On each level id can be a different type.
// For instance in embedded environment to minimize memory dynamic allocations a use of strings can be replace with fixed size char arrays.
// Each member of a hierarchy is expected to have a member of predefined type TypeName storing object id/name
// A repository for this traits class is an stl map

// Template parameter Types is a variadic sequence of all hierarchy types in given hierarchy
// from root to leaf.
// For for hierarchy A->B->C for instance it'll be instantiated as HirerachyTraits<A,B,C>
template <typename... Types>
struct HirerachyTraits {

    // The type Key is a tuple of ids to access a leaf object in its repository
    using Key = std::tuple<typename Types::TypeName...>;

    // Types parameters pack -> tuple of Types
    // Helps define the type of the leaf type in the hierarchy (see LastType)
    using TypesTuple = std::tuple<Types...>;

    // The leaf type in the given hierarchy
    // for hierarchy A->B->C it'll be C
    using LastType = typename std::tuple_element<sizeof...(Types)-1, TypesTuple>::type;

    // forward declaration of a comparator class for map repository
    struct TypeComparator;

    // The repository type
    using TypesMap = std::map<Key,LastType, TypeComparator>;


    // The comparator class comparing the keys
    // Each key is a tuple of a hierarchy type ids
    // A tuple can only be traversed recursively and that's what
    // compare method does. It starts from the root id and recursively calls compare
    // for a next type in the hierarchy chain if names at root level are the same.
    // The process is repeated all the way to the leaf object
    // Note: stl map considers keys k1 and k2 equal only if !(k1 < k2) and !(k2 < k1)
    struct TypeComparator
    {
        bool operator()(Key const& lhs , Key const& rhs) const
        {
            // Call compare for the root type
            return compare<0, Types...>(lhs, rhs);
        }

        template<std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<I == sizeof...(Tp), bool>::type
        compare(Key const& lhs , Key const& rhs) const
        {
            // Terminal condition. This compare is called for I == number of types in a Key tuple
            // We get here if values in the keys tuples are equal
            return false;
        }

        template<std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<I < sizeof...(Tp), bool>::type
        compare(Key const& lhs , Key const& rhs) const
        {
            // retrieve Ith values from the tuples on right and left side
            auto& lname = std::get<I>(lhs);
            auto& rname = std::get<I>(rhs);

            // compare them
            if(lname < rname) return true;
            if(rname < lname) return false;

            // they are equal keep going to the next tuple element
            return compare<I+1, Tp...>(lhs, rhs);

        }
    };


    // The keys used for iterating hierarchy objects

    // A start key value.
    // It has to be chosen so that it is less or equal than any other key
    // An assumption is made here that a tuple of empty names is less or equal than any other key
    static const Key& startKey() {
        static const Key START_KEY = std::make_tuple(typename Types::TypeName()...);
        return START_KEY;
    }

    // A predefiend value for endKey. This value is to be returned to indicate end of iteration
    static const Key& endKey() {
        static const Key END_KEY = std::make_tuple(typename Types::TypeName{"MagicEndKey"}...);
        return END_KEY;
    }

    // Generic processor utilities. The processor is intended to be called during a hierarchy iteration
    // with a function -parameter to be called on each object being iterated. A function arguments
    // are passed as variadic template arguments and expanded upon a call to the function.
    // The processor is generic and not bound to any particular processing function signature
    // The processor is also not bound to any particular hierarchy depth. The depth is resolved during a call
    // by expanding the tuple of keys.
    // However it is assumed that the processing function has the leaf type first in its parameters,
    // the sequence of all parent ids next and user arguments after.
    // For instance for hierarchy A->B->C a function to be called should look like
    // f(const C&, const A::TypeName&, const B::TypeName&, const C::TypeName&, args...)
    // The processor expands the call to the function on both members of the key tuple and variadic arguments
    // The variadic arguments of the function are expanded in template member process, the members of the key tuple
    // get expanded through partial Processor class template specialization for integer sequence parameter
    // The first template parameter is dummy and used. It is needed only to implement a partial template specialization
    // which is required if the template class is nested


    // Generic processor for all cases when 2nd template parameter is not an int sequence
    // Not used as is therefore both template parameters are dummy.
    // Only this class partial specialization with int sequence is used (see the next class)
    template<typename _, typename __>
    struct Processor { };

    // A partial processor specialization for 2nd template parameter int sequence
    template <typename _, int... S>
    struct Processor<_, seq::seq<S...> >
    {
        template <typename Function, typename... Args>
        static void process(Key const& key,LastType const& value, Function& f, Args&... args)
        {
            // The key gets expanded through int sequence template parameter,
            // the args are expanded as a parameter pack
            f(value, std::get<S>(key).data()..., args...);
        }
    };


    // A generic processing function.
    // It is a template parameterized by the function to call on a given object and that function custom arguments
    template <typename Function, typename... Args>
    static void process(Key const& key, LastType const& value, Function& f, Args&... args)
    {
        // This instantiates a Processor with 2nd template parameter as an integer sequence
        // generated from the key tuple size.
        // For instance for hierarchy A->B->C the integer sequence generated shall be seq::seq<0,1,2>
        Processor<int, typename seq::gens<std::tuple_size<Key>::value>::type>::process(key, value, f, args...);
    }

    // Batch iterator iterates over n leaf objects from a hierarchy starting from a key 'first'
    // On each object being iterated it call a processing function f.
    // At the end of the iteration it returns the key pointing to the next element Key(first+n) or endKey
    // if the iterator reached the end of the repository

    template <typename... Args>
    class BatchIterator
    {
        using HierarchyType = HirerachyTraits<Types...>;

        // The next two members are repository (map) and its iterator
        TypesMap const& typesMap;
        typename TypesMap::const_iterator iter;

        // Processing function arguments (Parameters pack converted to tuple)
        std::tuple<Args&...> args;
    public:
        BatchIterator(TypesMap const& tmap, Key const& first, Args&... argsParams) :
            typesMap(tmap),
            iter(endKey() == first ? tmap.end() : tmap.lower_bound(first)),
            args(argsParams...)
        {}

        // This method does the iteration over the map n elements
        // calling function f on each of them directly
        template <typename Function>
        typename HierarchyType::TypesMap::key_type handleNDirect(
                int n,
                Function& f // direct object function
                )
        {
            if(typesMap.end() == iter)
                return endKey();

            for( ;n > 0 && typesMap.end() != iter; ++iter, --n)
            {
                callfExtended(f, typename seq::gens<sizeof...(Args)>::type());
            }
            return typesMap.end() != iter ? iter->first : HierarchyType::endKey();
        }

        // This method does the iteration over the map n elements
        // calling function f through an adapter (intermediate exercise)
        template <typename Function>
        typename HierarchyType::TypesMap::key_type handleNWithTraitsAdapter(
                int n,
                Function& f // traits adapter function
                )
        {
            if(typesMap.end() == iter)
                return endKey();


            for( ;n > 0 && typesMap.end() != iter; ++iter, --n)
            {
                callf(f, typename seq::gens<sizeof...(Args)>::type());
            }
            return typesMap.end() != iter ? iter->first : HierarchyType::endKey();
        }

    private:
        // args extended in the call invocation,
        // the key tuple gets expanded inside the call to process function
        // process function shall call f on the object iter->second expanding both the key tuple elements
        // and args
        template <typename Function, int ...S>
        void callfExtended(Function& f, seq::seq<S...>)
        {
            HierarchyType::process(iter->first, iter->second, f, std::get<S>(args)...);
        }

        // Calls an adapter function f on an object being iterated
        // Expands args using int sequence
        template <typename Function, int ...S>
        void callf(Function& f, seq::seq<S...>)
        {
            f(iter->first, iter->second,std::get<S>(args)...);

        }

    };

};



#endif /* HITERATOR_HPP_ */
