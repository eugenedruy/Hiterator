
#include <string>
#include <ctime>
#include <iostream>
#include "intSequence.hpp"
#include <type_traits>
#include "Hiterator.hpp"



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




