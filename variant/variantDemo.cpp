#include<iostream>
#include<chrono>
#include<variant>

#include"variant.hpp"

class Type {
  public:
    Type() = default;
    ~Type() { std::cout << "Type::~Type()" << std::endl; }
    Type(const Type &)
    {
        std::cout << "Type::Type(const Type &)" << std::endl;
    }
    Type(Type &&)
    {
        std::cout << "Type::Type(Type &&)" << std::endl;
    }
    Type &operator=(const Type &)
    {
        std::cout << "Type::operator=(const Type &)" << std::endl;
        return *this;
    }
    Type &operator=(Type &&)
    {
        std::cout << "Type::operator=(Type &&)" << std::endl;
        return *this;
    }
    friend std::ostream &operator<<(std::ostream &o, const Type &t)
    { return o << "Type::operator<<"; }
};

int main()
{
    // two int ==> type int is not unique ==> error!
    // Variant<int, double, Type, char, int> v('c');
    Variant<int, double, Type, char> v = 'c';
    std::cout << *v.value<char>() << std::endl;
    v = 1.2;
    assert(!v.value<char>());
    std::cout << *v.value<double>() << std::endl;
    v = Type{};
    assert(!v.value<double>());
    assert(!v.value<0>());
    std::cout << *v.value<Type>() << std::endl;
    std::cout << *v.value<2>() << std::endl;
    // v = 1.2f;   //dont have param float ==> error!

    

    std::variant<int, double, Type, char> v2;
    auto begin = std::chrono::system_clock::now();
    decltype(begin) end;
    for(volatile size_t i = 0; i < 1e7; i++) {
        v = 1.1;
    }
    end = std::chrono::system_clock::now();
    std::cout << "cost " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << "ms" << std::endl;
    begin = std::chrono::system_clock::now();
    for(volatile size_t i = 0; i < 1e7; i++) {
        v2 = 1.1;
    }
    end = std::chrono::system_clock::now();
    std::cout << "cost " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << "ms" << std::endl;
    
}
