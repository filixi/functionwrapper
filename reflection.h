/* ----------------------------------------------------------------------------
** Copyright (c) 2017 Yifei LI, All Rights Reserved.
**
** reflection.h
** --------------------------------------------------------------------------*/

#ifndef _REFLECTION_H_
#define _REFLECTION_H_

#include <memory>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <vector>

#include "any.h"

namespace reflection {

using namespace any;

namespace detail {

template <class T>
struct NonConstReference {
  static constexpr int value = 0;
};

template <class T>
struct NonConstReference<const T &> {
  static constexpr int value = 0;
};

template <class T>
struct NonConstReference<T &> {
  static constexpr int value = 1;
};

template <class T>
struct NonConstReference<T &&> {
  static constexpr int value = 1;
};

template <class... Args>
constexpr int AnyNonConstReference(...) {
  return 0;
}

template <class T, class... Args>
constexpr int AnyNonConstReference(int) {
  return NonConstReference<T>::value + AnyNonConstReference<Args...>(0);
}

template <class T>
struct Forward {
  static T forward(T x) { return x; }
};

template <class T>
struct Forward<T &&> {
  template <class U>
  static auto forward(U &&x) { return std::move(x); }
};

template <class Fn, class... Args>
decltype(std::declval<Fn>()(std::declval<Args>()...)) RetType();

template <class Fn, class... Args, class = decltype(RetType<Fn, Args...>()) >
constexpr bool IsCallable(int) {
  return true;
}

template <class Fn, class... Args>
constexpr bool IsCallable(...) {
  return false;
}

} // namespace detail

template <class Fn, class Iterator>
auto InnerCall(Fn &fn, const Iterator &begin, const Iterator &end) {
  return fn();
}

template <class T, class... Args, class Fn, class Iterator>
auto InnerCall(Fn &fn, Iterator begin, const Iterator &end) {
  auto value = begin->template Get<std::decay_t<T> >(std::nothrow);
  if (value == nullptr)
    throw std::runtime_error("Argument Type Error!");
  
  auto recursive_bind = [&fn, &value](auto&&... rest){
    return fn(detail::Forward<T>::forward(const_cast<T &>(*value)),
              std::forward<Args>(rest)...);
  };
  
  return InnerCall<Args...>(recursive_bind, std::next(begin), end);
}

// return non void
template <
    class ReturnType,
    class... Args,
    class Fn,
    class ArgsContainer,
    class = std::enable_if_t<!std::is_same<void, ReturnType>::value, void> >
Any PreInnerCall(Fn &fn, ArgsContainer &&container) {
  return Any(InnerCall<Args...>(fn, container.begin(), container.end()));
}

// return void
template <
    class ReturnType,
    class... Args,
    class Fn,
    class ArgsContainer,
    class = std::enable_if_t<std::is_same<void, ReturnType>::value, void> >
Any PreInnerCall(Fn &fn, ArgsContainer &&container, ...) {
  InnerCall<Args...>(fn, container.begin(), container.end());
  return Any{};
}

template <class ArgsContainer>
class FunctionBasic {
 public:
  virtual Any Call(ArgsContainer &container) = 0;
  virtual Any Call(const ArgsContainer &container) = 0;
};

template <class ReturnType, class... Args>
class FunctionWithArgs {
 public:
  virtual ReturnType operator()(Args...) = 0;
};

template <class ArgsContainer, class Fn, class... Args>
class Function
  : public FunctionBasic<ArgsContainer>,
    public FunctionWithArgs<decltype(detail::RetType<Fn, Args...>()), Args...> {
 public:
  using ReturnType = decltype(detail::RetType<Fn, Args...>());
  Function(Fn fn) : fn_(fn) {}
  
  Any Call(ArgsContainer &container) override {
    if (sizeof...(Args) > container.size())
      throw std::runtime_error("Argument number Error!");
    
    return PreInnerCall<ReturnType, Args...>(fn_, container);
  }
  
  Any Call(const ArgsContainer &container) override {
    if (detail::AnyNonConstReference<Args...>(0))
      throw std::runtime_error("Const qualifer discarded!");
    if (sizeof...(Args) > container.size())
      throw std::runtime_error("Argument number Error!");
    
    return PreInnerCall<ReturnType, Args...>(fn_, container);
  }
  
  ReturnType operator()(Args... args) override {
    return fn_(std::forward<Args>(args)...);
  }
  
  operator std::function<ReturnType (Args...)>() {
    return std::function<ReturnType (Args...)>(fn_);
  }
  
 private:
  Fn fn_;
};

template <class ReturnType, class... Args>
auto FunctionWrapper(ReturnType (*fn)(Args...)) {
  return Function<std::vector<Any>, ReturnType (*)(Args...), Args...>(fn);
}

template <class... Args, class Fn>
auto GetWrapper(Fn &fn) {
  return Function<std::vector<Any>, Fn, Args...>(fn);
}

template <class... Args, class Fn>
auto FunctionWrapper(Fn &&fn) {
  static_assert(detail::IsCallable<Fn, Args...>(0),
                "Argument must be a callable object!");
  return GetWrapper<Args...>(fn);
}

} // namespace reflection

#endif // _REFLECTION_H_

