
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

struct City
{
    typedef std::array<char,80> TypeName;
    TypeName name;
    std::string zip; // zip code
    unsigned int id;

    /*
    template <typename CityIndexerKey>
    void setIndex(CityIndexerKey& key,  const std::string& cityName) {
        key.set
    }
    */

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
            char const* cityName,
            char const* carName)
    {
        ostr
        << " company " << rentalName
        << " city: " << cityName
        << " make: " << carName
        << " model: " << car.model.data()
        << std::endl;

    }
};



template <typename... Types>
struct StorageCommonTraits {

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
    static void print1(std::ostream& ostr, LastType const& value, Key const& t, seq::seq<S...>)
    {
        LastType::print(ostr, value, std::get<S>(t).data()...);
    }

    static void print(std::ostream& ostr, Key const& key, LastType const& value)
    {
        print1(ostr, value, key, typename seq::gens<sizeof...(Types)>::type());
    }


};


using Cars = StorageCommonTraits<RentalCompany, City, Car>;

using Cities = StorageCommonTraits<RentalCompany, City>;

using RentalCompanies = StorageCommonTraits<RentalCompany>;

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

/*
template <class Type, int ...S>
typename Type::TypesMap::key_type printN(std::ostream& ostr,
                                   typename Type::TypesMap const& typesMap,
                                   typename Type::TypesMap::key_type const& first,
                                   size_t n,
                                   seq::seq<S...>)
{
    if(Type::endKey() == first)
        return first;
    auto iter = typesMap.lower_bound(first);
    for( ;n > 0 && typesMap.end() != iter; ++iter, --n)
    {
        ostr
        << std::get<S>(iter->first).data()
        << std::endl;
    }
    return typesMap.end() != iter ? iter->first : Type::endKey();

}
*/

int main(int argc, char* argv[]) {

    Cities::Key ctkey11 = std::make_tuple
    (
        RentalCompany::TypeName{"EuroCar"},
        City::TypeName{"Paris"}
    );


    Car car1 = {"Renault", makeDate(2015), "r12345a"};
    Car car2 = {"Citroen", makeDate(2018), "c12345a"};
    Car car3 = {"Opel", makeDate(2017), "o12345a"};

    Cars::Key crkey111 = std::make_tuple
    (
        RentalCompany::TypeName{"EuroCar"},
        City::TypeName{"Paris"},
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
        City::TypeName{"London"},
        car4.make
    );

    cars.insert({crkey124, car4});

    Cars::Key crkey211 = std::make_tuple
    (
        RentalCompany::TypeName{"Avis"},
        City::TypeName{"Paris"},
        car2.make
    );

    cars.insert({crkey211, car2});

    Cars::Key crkey221 = std::make_tuple
    (
        RentalCompany::TypeName{"Avis"},
        City::TypeName{"London"},
        car3.make
    );


    cars.insert({crkey221, car3});

    Cars::Key crkey222 = std::make_tuple
    (
        RentalCompany::TypeName{"Avis"},
        City::TypeName{"London"},
        car2.make
    );

    cars.insert({crkey222, car2});

    Cars::Key crkey114 = std::make_tuple
    (
        RentalCompany::TypeName{"EuroCar"},
        City::TypeName{"Paris"},
        car4.make
    );

    cars.insert({crkey114, car4});

    for (auto const& carKeyVal : cars)
    {
        std::cout
            << " company " << std::get<0>(carKeyVal.first).data()
            << " city: " << std::get<1>(carKeyVal.first).data()
            << " make: " << carKeyVal.second.make.data()
            << " model: " << carKeyVal.second.model.data()
            << std::endl;
    }


    std::cout << "============== random access==============" << std::endl;
    auto iter = cars.lower_bound(crkey211);
    for (;cars.end() != iter;++iter)
    {
        std::cout
            << " company " << std::get<0>(iter->first).data()
            << " city: " << std::get<1>(iter->first).data()
            << " make: " << iter->second.make.data()
            << " model: " << iter->second.model.data()
            << std::endl;
    }

    std::cout << "============== from dummy key access==============" << std::endl;

    iter = cars.lower_bound(Cars::startKey());

    for (;cars.end() != iter;++iter)
    {
        std::cout
            << " company " << std::get<0>(iter->first).data()
            << " city: " << std::get<1>(iter->first).data()
            << " make: " << iter->second.make.data()
            << " model: " << iter->second.model.data()
            << std::endl;
    }

    std::cout << "============== use print==============" << std::endl;
    auto nextKey = Cars::startKey();
    int count  = 0;

    while(Cars::endKey() != nextKey)
    {
        nextKey = printN<Cars>(std::cout, cars, nextKey, 2);
        std::cout << "============== page " << ++count << " ==============" << std::endl;
    }



}




