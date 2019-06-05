
#include <string>
#include <ctime>
#include <map>
#include <array>
#include <tuple>
#include <algorithm>
#include <random>
#include <iostream>



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
    static const Key& emptyKey() {
        static const Key EMPTY_KEY = std::make_tuple(typename Types::TypeName()...);
        return EMPTY_KEY;
    }

    static const Key& endKey() {
        static const Key END_KEY = std::make_tuple(typename Types::TypeName{"MagicEndWord"}...);
        return END_KEY;
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
        ostr
        << " company " << std::get<0>(iter->first).data()
        << " city: " << std::get<1>(iter->first).data()
        << " make: " << iter->second.make.data()
        << " model: " << iter->second.model.data()
        << std::endl;
    }
    return typesMap.end() != iter ? iter->first : Type::endKey();

}

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

    iter = cars.lower_bound(Cars::emptyKey());

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
    auto nextKey = Cars::emptyKey();
    int count  = 0;

    while(Cars::endKey() != nextKey)
    {
        nextKey = printN<Cars>(std::cout, cars, nextKey, 2);
        std::cout << "============== page " << ++count << " ==============" << std::endl;
    }



}




