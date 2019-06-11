
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

template <typename... Types>
struct HirerachyTraits {

    using Key = std::tuple<typename Types::TypeName...>;
    // parameters pack -> tuple
    using TypesTuple = std::tuple<Types...>;

    struct TypeComparator
    {
        bool operator()(Key const& lhs , Key const& rhs) const
        {
            return compare<0, Types...>(lhs, rhs);
        }

        template<std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<I == sizeof...(Tp), bool>::type
        compare(Key const& lhs , Key const& rhs) const
        {
            return false;
        }

        template<std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<I < sizeof...(Tp), bool>::type
        compare(Key const& lhs , Key const& rhs) const
        {
            auto& lname = std::get<I>(lhs);
            auto& rname = std::get<I>(rhs);
            if(lname < rname) return true;
            if(rname < lname) return false;
            return compare<I+1, Tp...>(lhs, rhs);

        }
    };


    using LastType = typename std::tuple_element<sizeof...(Types)-1, TypesTuple>::type;

    using TypesMap = std::map<Key,LastType, TypeComparator>;

    // Has to be chosen so that it is less or equal than any other key
    static const Key& startKey() {
        static const Key START_KEY = std::make_tuple(typename Types::TypeName()...);
        return START_KEY;
    }

    static const Key& endKey() {
        static const Key END_KEY = std::make_tuple(typename Types::TypeName{"MagicEndKey"}...);
        return END_KEY;
    }

    template <int ...S>
    static void print(std::ostream& ostr, LastType const& value, Key const& t, seq::seq<S...>)
    {
        LastType::print(ostr, value, std::get<S>(t).data()...);
    }

    static void print(std::ostream& ostr, Key const& key, LastType const& value)
    {
        print(ostr, value, key, typename seq::gens<sizeof...(Types)>::type());
    }

    static void print(Key const& key, LastType const& value, std::ostream& ostr)
    {
        print(ostr, value, key, typename seq::gens<sizeof...(Types)>::type());
    }

    template<typename U, typename V>
    struct Processor { };

    template <typename U, int... S>
    struct Processor<U, seq::seq<S...> >
    {
        template <typename Function, typename... Args>
        static void process(Key const& key,LastType const& value, Function& f, Args&... args)
        {
            f(value, std::get<S>(key).data()..., args...);

        }
    };


    template <typename Function, typename... Args>
    static void process(Key const& key, LastType const& value, Function& f, Args&... args)
    {
        Processor<int, typename seq::gens<std::tuple_size<Key>::value>::type>::process(key, value, f, args...);
    }

    template <typename... Args>
    class BatchIterator
    {
        using Type = HirerachyTraits<Types...>;

        TypesMap const& typesMap;
        typename TypesMap::const_iterator iter;
        std::tuple<Args&...> args;
    public:
        BatchIterator(TypesMap const& tmap, Key const& first, Args&... argsParams) :
            typesMap(tmap),
            iter(endKey() == first ? tmap.end() : tmap.lower_bound(first)),
            args(argsParams...)
        {}

        template <typename Function>
        typename Type::TypesMap::key_type handleNWithTraitsAdapter(
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
            return typesMap.end() != iter ? iter->first : Type::endKey();
        }

        template <typename Function>
        typename Type::TypesMap::key_type handleNDirect(
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
            return typesMap.end() != iter ? iter->first : Type::endKey();
        }
    private:
        // Expands args using int sequence
        template <typename Function, int ...S>
        void callf(Function& f, seq::seq<S...>)
        {
            f(iter->first, iter->second,std::get<S>(args)...);

        }

        // args extended in the call invokation,
        // the key tuple gets expanded inside the call to process
        template <typename Function, int ...S>
        void callfExtended(Function& f, seq::seq<S...>)
        {
            Type::process(iter->first, iter->second, f, std::get<S>(args)...);
        }
    };
};


template <typename Container>
using ThePrintType = void(&)(typename Container::TypesMap::key_type const&,
                             typename Container::LastType const&,
                             std::ostream&);

template <typename Container, typename... Args>
void processNWithAdapter(typename Container::TypesMap const& theMap, ThePrintType<Container>& f, int n, Args&... args)
{
    int count  = 0;
    auto nextKey = Container::startKey();
    while(Container::endKey() != nextKey)
    {
        typename Container::template BatchIterator<Args...> iter(theMap, nextKey, args...);
        std::cout << "============== page " << ++count << " ==============" << std::endl;
        nextKey = iter.handleNWithTraitsAdapter(n,f);
    }

}

template <typename Container, typename F, typename... Args>
void processNDirect(typename Container::TypesMap const& theMap, F& f, int n, Args&... args)
{
    int count  = 0;
    auto nextKey = Container::startKey();
    while(Container::endKey() != nextKey)
    {
        typename Container::template BatchIterator<Args...> iter(theMap, nextKey, args...);
        std::cout << "============== page " << ++count << " ==============" << std::endl;
        nextKey = iter.handleNDirect(n,f);
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

template <class Type>
typename Type::TypesMap::key_type printN(std::ostream& ostr,
                                   typename Type::TypesMap const& typesMap,
                                   typename Type::TypesMap::key_type const& first,
                                   size_t n)
{
    if(Type::endKey() == first)
        return first;
    auto iter = typesMap.lower_bound(first);
    for( ;n > 0 && typesMap.end() != iter; ++iter, --n)
    {
        Type::print(ostr, iter->first, iter->second);
    }
    return typesMap.end() != iter ? iter->first : Type::endKey();

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



int main(int argc, char* argv[])
{

    populateCars(cars);
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
        nextCarKey = carsIter.handleNWithTraitsAdapter(4,(ThePrintType<Cars>)Cars::print);
    }


    std::cout << "============== use iterator for offices==============" << std::endl;

    count  = 0;
    auto nextOfficeKey = Offices::startKey();
    while(Offices::endKey() != nextOfficeKey)
    {
        Offices::BatchIterator<std::ostream> officesIter(offices, nextOfficeKey, std::cout);
        std::cout << "============== page " << ++count << " ==============" << std::endl;
        nextOfficeKey = officesIter.handleNWithTraitsAdapter(3,(ThePrintType<Offices>)Offices::print);
    }


    std::cout << "============== use generic processor with traits adapter for offices==============" << std::endl;
    processNWithAdapter<Offices>(offices, (ThePrintType<Offices>)Offices::print, 3, std::cout);

    using OfficePrint = void(&)(const Office&, char const*, char const*, std::ostream&);


    std::cout << "============== use generic processor parameterized with LastType function for offices==============" << std::endl;
    processNDirect<Offices>(offices, (OfficePrint)Office::print, 3, std::cout);

}




