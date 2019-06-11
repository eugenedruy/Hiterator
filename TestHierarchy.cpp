
#include <string>
#include <ctime>
#include <map>
#include <array>
#include <tuple>
#include <algorithm>
#include <random>
#include <iostream>
#include "intSequence.hpp"
#include <type_traits>



struct RentalCompany
{
    typedef std::array<char,64> TypeName;
    TypeName name;
    unsigned int id;
};

struct Office
{
    typedef std::array<char,80> TypeName;
    TypeName name;
    std::array <char,32> zip; // zip code
    unsigned int id;

    static void print(
            std::ostream& ostr,
            const Office& office,
            char const* rentalName,
            char const* officeName)
    {
        ostr
        << " company " << rentalName
        << " city: " << officeName
        << " zip: " << office.zip.data()
        << std::endl;

    }

    static void print(
            const Office& office,
            char const* rentalName,
            char const* officeName,
            std::ostream& ostr)
    {
        ostr
        << " company " << rentalName
        << " city: " << officeName
        << " zip: " << office.zip.data()
        << std::endl;

    }
};


struct Car
{
    typedef std::array <char,256> TypeName;
    TypeName make;
    struct tm makeDate;
    std::array <char,64> model;

    static void print(
            std::ostream& ostr,
            const Car& car,
            char const* rentalName,
            char const* officeName,
            char const* carName)
    {
        ostr
        << " company " << rentalName
        << " city: " << officeName
        << " make: " << carName
        << " model: " << car.model.data()
        << std::endl;

    }

    static void print(
            const Car& car,
            char const* rentalName,
            char const* officeName,
            char const* carName,
            std::ostream& ostr)
    {
        print(ostr, car, rentalName, officeName, carName);
    }

    static void print(
            std::ostream& ostr,
            const Car& car,
            char const* rentalName,
            char const* officeName,
            char const* carName,
            char const* rentalName1,
            char const* officeName1,
            char const* carName1
            )
    {
        ostr
        << " print 6 "
        << " company " << rentalName
        << " city: " << officeName
        << " make: " << carName
        << " model: " << car.model.data()
        << std::endl;

    }


};

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

template<typename HierarchyType>
struct Adapters
{
    // Next 3 methods are just adapters that were used in th intermediary exercise

    using LastType = typename HierarchyType::LastType;
    using Key = typename HierarchyType::Key;
    using TypesTuple = typename HierarchyType::TypesTuple;

    template <int ...S>
    static void print(std::ostream& ostr, LastType const& value, Key const& t, seq::seq<S...>)
    {
        HierarchyType::LastType::print(ostr, value, std::get<S>(t).data()...);
    }

    static void print(std::ostream& ostr, Key const& key, LastType const& value)
    {
        print(ostr, value, key, typename seq::gens<std::tuple_size<TypesTuple>::value>::type());
    }

    static void print(Key const& key, LastType const& value, std::ostream& ostr)
    {
        print(ostr, value, key, typename seq::gens<std::tuple_size<TypesTuple>::value>::type());
    }

};


// A wrapper doing batch processing of leaf objects in a hierarchy
// It instantiates a BatchIterator and call function f on each object being iterated
template <typename HierarchyType, typename F, typename... Args>
void processNDirect(typename HierarchyType::TypesMap const& theMap, F& f, int n, Args&... args)
{
    int count  = 0;
    auto nextKey = HierarchyType::startKey();
    while(HierarchyType::endKey() != nextKey)
    {
        typename HierarchyType::template BatchIterator<Args...> iter(theMap, nextKey, args...);
        std::cout << "============== page " << ++count << " ==============" << std::endl;
        nextKey = iter.handleNDirect(n,f);
    }

}


// typedef of a reference to a  function
template <typename HierarchyType>
using ThePrintType = void(&)(typename HierarchyType::TypesMap::key_type const&,
                             typename HierarchyType::LastType const&,
                             std::ostream&);

// A wrapper doing batch processing of leaf objects in a hierarchy
// It instantiates a BatchIterator and call an adapter function f on each object being iterated
template <typename HierarchyType, typename... Args>
void processNWithAdapter(typename HierarchyType::TypesMap const& theMap, ThePrintType<HierarchyType>& f, int n, Args&... args)
{
    int count  = 0;
    auto nextKey = HierarchyType::startKey();
    while(HierarchyType::endKey() != nextKey)
    {
        typename HierarchyType::template BatchIterator<Args...> iter(theMap, nextKey, args...);
        std::cout << "============== page " << ++count << " ==============" << std::endl;
        nextKey = iter.handleNWithTraitsAdapter(n,f);
    }

}


using Cars = HirerachyTraits<RentalCompany, Office, Car>;

using Offices = HirerachyTraits<RentalCompany, Office>;

using RentalCompanies = HirerachyTraits<RentalCompany>;

struct tm makeDate(int year,
                   int month// months since jan
                   )
{
    return  {0,0,0,0,0,month,year-1900,0,0};
}

struct tm makeDate(int year)
{
    return makeDate(year,0);
}



static Cars::TypesMap cars;
static Offices::TypesMap offices;

template <class HierarchyType>
typename HierarchyType::TypesMap::key_type printN(std::ostream& ostr,
                                   typename HierarchyType::TypesMap const& typesMap,
                                   typename HierarchyType::TypesMap::key_type const& first,
                                   size_t n)
{
    if(HierarchyType::endKey() == first)
        return first;
    auto iter = typesMap.lower_bound(first);
    for( ;n > 0 && typesMap.end() != iter; ++iter, --n)
    {
        Adapters<HierarchyType>::print(ostr, iter->first, iter->second);
    }
    return typesMap.end() != iter ? iter->first : HierarchyType::endKey();

}

void populateCars(Cars::TypesMap& cars)
{
    Car car1 = {"Renault", makeDate(2015), "r12345a"};
    Car car2 = {"Citroen", makeDate(2018), "c12345a"};
    Car car3 = {"Opel", makeDate(2017), "o12345a"};

    Cars::Key crkey111 = std::make_tuple
    (
        RentalCompany::TypeName{"EuroCar"},
        Office::TypeName{"Paris"},
        car1.make
    );


    cars.insert({crkey111, car1});

    Car::TypeName makes[] = {"Ford", "Renault", "Citroen", "Opel", "Audi"};



    std::default_random_engine generator(1234567890);
    std::uniform_int_distribution<int> distribution(0,sizeof(makes)/sizeof(makes[0]) - 1);
    int r = distribution(generator);

    std::cout << "models size " << sizeof(makes)/sizeof(makes[0]) << " r: " << r << std::endl;
    Car::TypeName randomMake = makes[r];

    Car car4 = {randomMake, makeDate(2017), "?12345a"};


    Cars::Key crkey124 = std::make_tuple
    (
        RentalCompany::TypeName{"EuroCar"},
        Office::TypeName{"London"},
        car4.make
    );

    cars.insert({crkey124, car4});

    Cars::Key crkey211 = std::make_tuple
    (
        RentalCompany::TypeName{"Avis"},
        Office::TypeName{"Paris"},
        car2.make
    );

    cars.insert({crkey211, car2});

    Cars::Key crkey221 = std::make_tuple
    (
        RentalCompany::TypeName{"Avis"},
        Office::TypeName{"London"},
        car3.make
    );


    cars.insert({crkey221, car3});

    Cars::Key crkey222 = std::make_tuple
    (
        RentalCompany::TypeName{"Avis"},
        Office::TypeName{"London"},
        car2.make
    );

    cars.insert({crkey222, car2});

    Cars::Key crkey114 = std::make_tuple
    (
        RentalCompany::TypeName{"EuroCar"},
        Office::TypeName{"Paris"},
        car4.make
    );

    cars.insert({crkey114, car4});

}

void populateOffices(Offices::TypesMap& offices)
{

    unsigned int id = 1;
    Office office1 = {"Paris", "paris_zip", id++};
    Office office2 = {"London", "london_zip", id++};
    Office office3 = {"Boston", "boston_zip", id++};

    Offices::Key ofckey11 = std::make_tuple
    (
        RentalCompany::TypeName{"EuroCar"},
        office1.name
    );


    offices.insert({ofckey11, office1});


    Office office4 = {"Seattle", "seattle_zip", id++};


    Offices::Key ofckey124 = std::make_tuple
    (
        RentalCompany::TypeName{"EuroCar"},
        office4.name
    );

    offices.insert({ofckey124, office4});

    Offices::Key ofckey211 = std::make_tuple
    (
        RentalCompany::TypeName{"Avis"},
        office2.name
    );

    offices.insert({ofckey211, office2});

    Offices::Key ofckey221 = std::make_tuple
    (
        RentalCompany::TypeName{"Avis"},
        office3.name
    );


    offices.insert({ofckey221, office3});

    Offices::Key ofckey222 = std::make_tuple
    (
        RentalCompany::TypeName{"Avis"},
        office2.name
    );

    offices.insert({ofckey222, office2});

    Offices::Key ofckey114 = std::make_tuple
    (
        RentalCompany::TypeName{"Hertz"},
        office4.name
    );

    offices.insert({ofckey114, office4});
}



// Just running test cases
int main(int argc, char* argv[])
{

    // Sample hierarchies:
    // Rentals->Offices->Cars
    //  and
    // Offices->Cars

    // Populate a hierarchy of cars objects (depth of 3)
    populateCars(cars);
    // Populate a hierarchy of office objects (depth of 2)
    populateOffices(offices);

    std::cout << "============== use print==============" << std::endl;
    auto nextKey = Cars::startKey();
    int count  = 0;

    while(Cars::endKey() != nextKey)
    {
        std::cout << "============== page " << ++count << " ==============" << std::endl;
        nextKey = printN<Cars>(std::cout, cars, nextKey, 2);
    }

    std::cout << "============== use iterator==============" << std::endl;

    count  = 0;
    auto nextCarKey = Cars::startKey();
    while(Cars::endKey() != nextCarKey)
    {
        Cars::BatchIterator<std::ostream> carsIter(cars, nextCarKey, std::cout);
        std::cout << "============== page " << ++count << " ==============" << std::endl;
        nextCarKey = carsIter.handleNWithTraitsAdapter(4,(ThePrintType<Cars>)Adapters<Cars>::print);
    }


    std::cout << "============== use iterator for offices==============" << std::endl;

    count  = 0;
    auto nextOfficeKey = Offices::startKey();
    while(Offices::endKey() != nextOfficeKey)
    {
        Offices::BatchIterator<std::ostream> officesIter(offices, nextOfficeKey, std::cout);
        std::cout << "============== page " << ++count << " ==============" << std::endl;
        nextOfficeKey = officesIter.handleNWithTraitsAdapter(3,(ThePrintType<Offices>)Adapters<Offices>::print);
    }


    std::cout << "============== use generic processor with traits adapter for offices==============" << std::endl;
    processNWithAdapter<Offices>(offices, (ThePrintType<Offices>)Adapters<Offices>::print, 3, std::cout);

    using OfficePrint = void(&)(const Office&, char const*, char const*, std::ostream&);


    std::cout << "============== use generic processor parameterized with LastType function for offices==============" << std::endl;
    processNDirect<Offices>(offices, (OfficePrint)Office::print, 3, std::cout);

}




