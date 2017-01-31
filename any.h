/* ----------------------------------------------------------------------------
** Copyright (c) 2017 Yifei LI, All Rights Reserved.
**
** any.h
** --------------------------------------------------------------------------*/

#ifndef _REFLECTION_ANY_H_
#define _REFLECTION_ANY_H_

#include <exception>
#include <functional>
#include <initializer_list>
#include <typeindex>

namespace any {

class Any {
 public:
  Any() : any_(nullptr), index_(typeid(void)) {}
  
  Any(const Any &) = delete;
  
  template <class T>
  Any(T &&x)
    : any_(new std::decay_t<T>(std::forward<T>(x))),
      index_(typeid(std::decay_t<T>)),
      deleter_(Deleter<std::decay_t<T> >) {}
  
  template <class T, class... Args>
  Any(Args&&... args)
    : any_(new T(std::forward<Args>(args)...)), index_(typeid(T)),
      deleter_(Deleter<T>) {}
  
  template <class T, class U>
  Any(const std::initializer_list<U> &list)
    : any_(new T(list)), index_(typeid(T)), deleter_(Deleter<T>) {}
  
  Any(Any &&x) : any_(x.any_), index_(x.index_), deleter_(x.deleter_) {
    x.any_ = nullptr;
  }
  
  Any &operator=(const Any &) = delete;
  
  Any &operator=(Any &&x) noexcept {
    auto temp = x.any_;
    x.any_ = nullptr;
    any_ = temp;
    
    index_ = x.index_;
    deleter_ = x.deleter_;
    return *this;
  }

  template <class T>
  T &operator=(T &&x) {
    if (any_)
      deleter_(any_);
    any_ = new std::decay_t<T>(std::forward<T>(x));
    index_ = typeid(std::decay_t<T>);
    deleter_ = Deleter<std::decay_t<T> >;
    return Get<T>();
  }
  
  ~Any() {
    if (any_)
      deleter_(any_);
  }
  
  template <class T>
  static void Deleter(void *x) {
    delete (T *)x;
  }
 
  template <class T>
  T &Get() {
    if (index_ == typeid(T))
      return *(T *)any_;
    throw std::runtime_error("Bad cast");
  }
  
  template <class T>
  const T &Get() const {
    if (index_ == typeid(T))
      return *(const T *)any_;
    throw std::runtime_error("Bad cast");
  }
  
  template <class T>
  T *Get(const std::nothrow_t &) noexcept {
    return index_ == typeid(T) ? (T *)any_ : nullptr;
  }
  
  template <class T>
  const T *Get(const std::nothrow_t &) const noexcept {
    return index_ == typeid(T) ? (const T *)any_ : nullptr;
  }
  
  bool Empty() const {
    return any_ == nullptr;
  }
  
 private:
  void *any_ = nullptr;
  std::type_index index_;
  std::function<void (void *)> deleter_;
};

} // namespace any

#endif // _REFLECTION_ANY_H_

