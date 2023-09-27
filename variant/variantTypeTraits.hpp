#ifndef _VARIANT_TYPE_TRAITS_HPP__
#define _VARIANT_TYPE_TRAITS_HPP__

#include<algorithm>
#include<typeinfo>
#include<cassert>

#include "myTypeTraits.hpp"

namespace vtt
{

template<typename U, typename ...T>
struct TypeDeleter;

namespace detail
{
template<typename Goal, typename ...T>
struct TypeNum;
template<typename Goal>
struct TypeNum<Goal> {
    static constexpr size_t value = 0;
};
template<typename Goal, typename U, typename ...T>
struct TypeNum<Goal, U, T...> {
    static constexpr size_t value =
        TypeNum<Goal, T...>::value +
        (size_t)std::is_same<Goal, U>::value;
};

template<size_t N, typename Goal, typename ...T>
struct TypeNumIs {
    static constexpr bool value = TypeNum<Goal, T...>::value == N;
};

//**INDENT-OFF**
template<size_t N, typename ...T>
struct TypeIsUnique {
    static_assert(N < sizeof...(T), "TypeIsUnique: index is too much");
    static constexpr bool value = TypeNumIs<1, getTypeN_t<N, T...>, T...>::value;
};

template<size_t N, typename ...T>
struct TypesIsUniqueHelper {
    static constexpr size_t fn()
    {
        return TypeIsUnique<N, T...>::value +
               TypesIsUniqueHelper<N - 1, T...>::fn();
    }
};
template<typename ...T>
struct TypesIsUniqueHelper<0, T...> {
    static constexpr size_t fn()
    { return TypeIsUnique<0, T...>::value; }
};

template<typename G, typename U, typename ...T>
struct GetTypeIndexHelper {
    static constexpr size_t fn() {
        return std::is_same<G, U>::value ? 
            0 : 1 + GetTypeIndexHelper<G, T...>::fn();
    }
};
template<typename G, typename U>
struct GetTypeIndexHelper<G, U> {
    static constexpr size_t fn() {
        //when using, not same<G, U> should not happen
        return std::is_same<G, U>::value ? 0 : 1;
    }
};

template<typename T>
void CopyAssign(T *dst, const T &a)
{
    *dst = a;
}
template<typename Assign, typename T>
void CopyAssign(T *dst, const Assign &a,
    typename std::enable_if<!std::is_same<Assign, T>::value>* = nullptr)
{
    TypeDeleter<T>::fn(0, dst);
    new(dst) Assign(a);
}

template<typename T>
void MoveAssign(T *dst, T &&a)
{
    *dst = std::move(a);
}
template<typename Assign, typename T>
void MoveAssign(T *dst, Assign &&a, 
    typename std::enable_if<!std::is_same<Assign, T>::value>* = nullptr)
{
    TypeDeleter<T>::fn(0, dst);
    new(dst) Assign(std::move(a));
}

template<typename T>
constexpr const T &max(const T &t1, const T &t2) { return t1 > t2 ? t1 : t2; }

//**INDENT-ON**
}

/****************** type unique type traits *****************/
//**INDENT-OFF**
template<typename ...T>
struct TypesIsUnique {
    static constexpr bool value =
        detail::TypesIsUniqueHelper<sizeof...(T) - 1, T...>::fn() == sizeof...(T);
};
template<>
struct TypesIsUnique<> {
    static constexpr bool value = false;
};
//**INDENT-ON**
// constexpr bool bbb = TypesIsUnique<int, float, double, char, double>::value;
// ==> false(two double)


/****************** template param list calc memory most used *****************/
template<typename U, typename ...T>
struct TypesMemoryMax {
    static constexpr size_t value =
        detail::max(sizeof(U), TypesMemoryMax<T...>::value);
};
template<typename T>
struct TypesMemoryMax<T> {
    static constexpr size_t value = sizeof(T);
};
// constexpr auto sss = TypesMemoryMax<int, double, long double, int[8]>::value;
// ==> sizeof(int[8])

/****************** type_info get by index *****************/
template<typename U, typename ...T>
struct TypeInfoGetLoop {
    static const std::type_info &fn(size_t idx)
    {
        if(idx == 0) {
            return typeid(U);
        }
        return TypeInfoGetLoop<T...>::fn(idx - 1);
    }
};
template<typename U>
struct TypeInfoGetLoop<U> {
    static const std::type_info &fn(size_t idx)
    {
        assert(idx == 0);
        return typeid(U);
    }
};

/****************** deleter by idx *****************/
template<typename U, typename ...T>
struct TypeDeleter {
    static void fn(size_t idx, void *pdata)
    {
        if(idx == 0) {
            ((U *)pdata)->~U();
            return;
        }
        TypeDeleter<T...>::fn(idx - 1, pdata);
    }
};
template<typename U>
struct TypeDeleter<U> {
    static void fn(size_t idx, void *pdata)
    {
        assert(idx == 0);
        ((U *)pdata)->~U();
    }
};

/****************** copy by idx *****************/
template<typename U, typename ...T>
struct TypeCopyConstruct {
    static void fn(size_t idx, void *dst, const void *src)
    {
        if(idx == 0) {
            new(dst) U(*static_cast<const U *>(src));
            return;
        }
        TypeCopyConstruct<T...>::fn(idx - 1, dst, src);
    }
};
template<typename U>
struct TypeCopyConstruct<U> {
    static void fn(size_t idx, void *dst, const void *src)
    {
        assert(idx == 0);
        new(dst) U(*static_cast<const U *>(src));
        return;
    }
};

/****************** operator=(Assign) by idx *****************/
template<typename AssignType, typename U, typename ...T>
struct TypeCopyAssign {
    static void fn(const AssignType &a, size_t idx, void *dst)
    {
        if(idx == 0) {
            detail::CopyAssign<AssignType, U>((U *)dst, a);
            return;
        }
        TypeCopyAssign<AssignType, T...>::fn(a, idx - 1, dst);
    }
};
template<typename AssignType, typename U>
struct TypeCopyAssign<AssignType, U> {
    static void fn(const AssignType &a, size_t idx, void *dst)
    {
        assert(idx == 0);
        detail::CopyAssign<AssignType, U>((U *)dst, a);
    }
};

/****************** operator=(Assign) by idx *****************/
template<typename AssignType, typename U, typename ...T>
struct TypeMoveAssign {
    static void fn(AssignType &&a, size_t idx, void *dst)
    {
        if(idx == 0) {
            detail::MoveAssign<AssignType, U>((U *)dst, std::move(a));
            return;
        }
        TypeMoveAssign<AssignType, T...>::fn(std::move(a), idx - 1, dst);
    }
};
template<typename AssignType, typename U>
struct TypeMoveAssign<AssignType, U> {
    static void fn(AssignType &&a, size_t idx, void *dst)
    {
        assert(idx == 0);
        detail::MoveAssign<AssignType, U>((U *)dst, std::move(a));
    }
};

/**************** type is in template param list **************/
template<typename G, typename ...T>
struct TypeIsIn {
    static constexpr bool value = detail::TypeNumIs<1, G, T...>::value;
};
// constexpr auto bbb = TypeIsIn<char, int, double, char>::value;
// ==> true

/************* get type index in template param list **********/
template<typename G, typename ...T>
struct GetTypeIndex {
    static constexpr size_t value = detail::GetTypeIndexHelper<G, T...>::fn();
};
// constexpr auto bbb = GetTypeIndex<char, int, double, char>::value;
// ==> 2


}
#endif
