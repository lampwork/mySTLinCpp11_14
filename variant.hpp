#ifndef _VARIANT_HPP__
#define _VARIANT_HPP__

#include <memory>
#include <typeinfo>

#include "variantTypeTraits.hpp"


#define REQ(...) \
    typename = typename std::enable_if<(__VA_ARGS__)>::type
#define REQ_TYPE typename

template<typename ...Tp>
class Variant {
  public:
    static_assert(sizeof...(Tp) > 0, "template param list cannot be empty!");
    static_assert(vtt::TypesIsUnique<Tp...>::value, "template param list must be unique!");

  public:
    template<typename U, REQ(vtt::TypeIsIn<U, Tp...>::value)>
    Variant(const U &t);
    template<typename U, REQ(vtt::TypeIsIn<U, Tp...>::value)>
    Variant(U &&t);
    Variant(const Variant &a);
    Variant(Variant &&a);

    ~Variant()
    {
        vtt::TypeDeleter<Tp...>::fn(typeIdx, pdata);
        free(pdata);
    }

    template<typename U, REQ(vtt::TypeIsIn<U, Tp...>::value)>
    Variant &operator=(const U &t);
    template<typename U, REQ(vtt::TypeIsIn<U, Tp...>::value)>
    Variant &operator=(U &&t);
  public:   //type info functions
    template<typename U, REQ(vtt::TypeIsIn<U, Tp...>::value)>
    static constexpr size_t TypeIdxOf() { return vtt::GetTypeIndex<U, Tp...>::value; };
  public:   //get value functions
    size_t TypeIndex() const { return typeIdx; }
    template<typename U, REQ(vtt::TypeIsIn<U, Tp...>::value)>
    const U *value() const;
    template<size_t Idx, typename U = getTypeN_t<Idx, Tp...>>
    const U * value() const;
    template<typename U, REQ(vtt::TypeIsIn<U, Tp...>::value)>
    U *value();
    template<size_t Idx, typename U = getTypeN_t<Idx, Tp...>>
    U * value();
  private:
    static const std::type_info &GetTypeInfo(size_t idx);
  private:
    ssize_t typeIdx = -1;
    void *pdata;
};


/****************** implement ***************/
template<typename ...Tp>
template<typename U, REQ_TYPE>
Variant<Tp...>::Variant(const U &t)
    : typeIdx(vtt::GetTypeIndex<U, Tp...>::value),
      pdata(malloc(vtt::TypesMemoryMax<Tp...>::value))
{
    new(pdata) U(t);
}
template<typename ...Tp>
template<typename U, REQ_TYPE>
Variant<Tp...>::Variant(U &&t)
    : typeIdx(vtt::GetTypeIndex<U, Tp...>::value),
      pdata(malloc(vtt::TypesMemoryMax<Tp...>::value))
{
    new(pdata) U(std::move(t));
}
template<typename ...Tp>
Variant<Tp...>::Variant(const Variant &a)
    : typeIdx(a.typeIdx),
      pdata(malloc(vtt::TypesMemoryMax<Tp...>::value))
{
    vtt::TypeCopyConstruct<Tp...>::fn(typeIdx, pdata, a.pdata);
}
template<typename ...Tp>
Variant<Tp...>::Variant(Variant &&a)
    : typeIdx(a.typeIdx), pdata(a.pdata)
{
    a.typeIdx = -1;
    a.pdata = nullptr;
}

template<typename ...Tp>
template<typename U, REQ_TYPE>
Variant<Tp...> &Variant<Tp...>::operator=(const U &t)
{
    vtt::TypeCopyAssign<U, Tp...>::fn(t, typeIdx, pdata);
    typeIdx = vtt::GetTypeIndex<U, Tp...>::value;
    return *this;
}
template<typename ...Tp>
template<typename U, REQ_TYPE>
Variant<Tp...> &Variant<Tp...>::operator=(U &&t)
{
    vtt::TypeMoveAssign<U, Tp...>::fn(std::move(t), typeIdx, pdata);
    typeIdx = vtt::GetTypeIndex<U, Tp...>::value;
    return *this;
}

template<typename ...Tp>
template<typename U, REQ_TYPE>
const U *Variant<Tp...>::value() const
{
    if(GetTypeInfo(typeIdx) == typeid(U)) {
        return (const U *)pdata;
    } else {
        return nullptr;
    }
};
template<typename ...Tp>
template<size_t Idx, typename U>
const U *Variant<Tp...>::value() const
{
    if(typeIdx == Idx) {
        return (const getTypeN_t<Idx, Tp...> *)pdata;
    } else {
        return nullptr;
    }
}
template<typename ...Tp>
template<typename U, REQ_TYPE>
U *Variant<Tp...>::value()
{
    if(GetTypeInfo(typeIdx) == typeid(U)) {
        return (U *)pdata;
    } else {
        return nullptr;
    }
};
template<typename ...Tp>
template<size_t Idx, typename U>
U *Variant<Tp...>::value()
{
    if(typeIdx == Idx) {
        return (getTypeN_t<Idx, Tp...> *)pdata;
    } else {
        return nullptr;
    }
}

template<typename ...Tp>
const std::type_info &Variant<Tp...>::GetTypeInfo(size_t idx)
{ return vtt::TypeInfoGetLoop<Tp...>::fn(idx); }

#undef REQ_TYPE
#undef REQ
#endif
