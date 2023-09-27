#include<cassert>
#include<cstring>
#include<any>
#include<chrono>
#include<iostream>

#include "any.hpp"

struct Type1 {
    Type1() = default;
    Type1(const Type1 &) = default;
    Type1(Type1 &&) = delete;
    Type1 &operator=(const Type1 &) = default;
    Type1 &operator=(Type1 &&) = delete;
    int i = 1;
};

struct Type2 {
    Type2() = default;
    Type2(const Type2 &) = delete;
    Type2(Type2 &&) = default;
    Type2 &operator=(const Type2 &) = delete;
    Type2 &operator=(Type2 &&) = default;
    int i = 2;
};

int main()
{
    tjh::Any any = 10, any2 = "123";
    assert(!any.value<double>());
    auto pa1 = any.value<int>();
    assert(pa1 && *pa1 == 10);
    any = "1234";
    auto pa2 = any.value<const char *>();
    assert(pa2 && !strcmp(*pa2, "1234"));
    any = (const char *[]) {"1", "12", "123"};
    auto pa3 = any.value<const char **>();
    assert(pa3 && !strcmp((*pa3)[0], "1"));
    assert(pa3 && !strcmp((*pa3)[1], "12"));
    assert(pa3 && !strcmp((*pa3)[2], "123"));
    Type1 t1;
    any = t1;
    auto pa4 = any.value<Type1>();
    assert(pa4 && pa4->i == 1);
    Type2 t2;
    any = std::move(t2);
    auto pa5 = any.value<Type2>();
    assert(pa5 && pa5->i == 2);

    std::any any3;
    auto begin = std::chrono::system_clock::now();
    decltype(begin) end;
    for(volatile size_t i = 0; i < 1e7; i++) {
        any = 1.1;
    }
    end = std::chrono::system_clock::now();
    std::cout << "cost " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << "ms" << std::endl;
    begin = std::chrono::system_clock::now();
    for(volatile size_t i = 0; i < 1e7; i++) {
        any3 = 1.1;
    }
    end = std::chrono::system_clock::now();
    std::cout << "cost " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << "ms" << std::endl;
}
