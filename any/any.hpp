#ifndef _TJH_TOY_HPP__
#define _TJH_TOY_HPP__

#include<typeinfo>
#include<utility>
#include<memory>
#include<type_traits>

namespace tjh
{
namespace detail__
{
template<bool b, typename U = void>
using enable_if_t = typename std::enable_if<b, U>::type;
template<typename T>
using decay_t = typename std::decay<T>::type;
template<typename T>
using rm_ref_t = typename std::remove_reference<T>::type;
template<typename T>
using rm_cvref_t = typename std::remove_cv<rm_ref_t<T>>::type;

template<typename T>
struct ArrToPtr { using type = T; };
template<typename T, size_t N>
struct ArrToPtr<T[N]> { using type = typename ArrToPtr<T>::type *; };
template<typename T>
using ArrToPtr_t = typename std::conditional <
                   std::is_array<typename std::remove_reference<T>::type>::value,
                   typename ArrToPtr<typename std::remove_reference<T>::type>::type, //is array or array ref
                   T >::type;

template<typename T>
using any_decay_t = rm_cvref_t<ArrToPtr_t<T>>;

};

class Any {
    struct ImplBase {
        virtual void *value() = 0;
        virtual ~ImplBase() = default;
        virtual const std::type_info &TypeInfo() const = 0;
    };
    template<typename T>
    struct Impl : public ImplBase {
        //specialize for move and copy(if cannot move)
        //**INDENT-OFF**
        template<typename U, typename = detail__::enable_if_t<std::is_constructible<T, U&&>::value>>
        Impl(U && t): p(new T(std::forward<U>(t))) {}
        //**INDENT-ON**
        ~Impl() override { delete p; }
        void *value() override { return (void *)p; }
        const std::type_info &TypeInfo() const override { return typeid(T); }
        T *p;
    };
  public:
    Any(): impl(nullptr) {}
    template<typename T>
    using decay_t = typename std::decay<T>::type;
    template<typename T, typename Type = detail__::ArrToPtr_t<T>>
    Any(const T &t): impl(new Impl<Type>((Type)t)) {}
    template<typename T, typename Type = detail__::ArrToPtr_t<T>>
    Any(T && t): impl(new Impl<Type>(std::move((Type)t))) {}
    Any(const Any &a) = delete;
    Any(Any &&a) = default;
    ~Any() = default;

    Any &operator=(const Any &a) = delete;
    Any &operator=(Any &&a) = default;

    template<typename T, typename Type = detail__::any_decay_t<T>>
    auto operator=(T && t) -> decltype(*(Type *)nullptr);
  public:
    template<typename T>
    bool TypeInfo() const { return typeid(T) == impl->TypeInfo(); };
    bool Valid() const { return (bool)impl; }
    template<typename T>
    T *value()
    {
        if(impl && impl->TypeInfo() == typeid(T)) {
            return (T *)(impl->value());
        } else {
            return nullptr;
        }
    };
    template<typename T>
    T &value_or(const T &t)
    {
        if(impl && impl->TypeInfo() == typeid(T)) {
            return *(T *)(impl->value());
        } else {
            return const_cast<T &>(t);
        }
    };
  private:
    std::unique_ptr<ImplBase> impl;
};

template<typename T, typename Type = detail__::any_decay_t<T>>
auto Any::operator=(T && t) -> decltype(*(Type *)nullptr)
{
    //if T is ref ==> Type remove ref
    //if T is array or array ref ==> Type remove ref and convert array to pointer
    //if T has cv qualifier ==> Type remove cv
    auto pimpl = dynamic_cast<Impl<Type> *>(impl.get());
    if(!pimpl) {
        impl.reset(new Impl<Type>(std::forward<T>(t)));
    } else {
        *((Type *)pimpl->value()) = std::forward<T>(t);
    }
    return *(Type *)impl->value();
}


}



#endif
