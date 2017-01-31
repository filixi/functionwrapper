/* ----------------------------------------------------------------------------
** Copyright (c) 2017 Yifei LI, All Rights Reserved.
**
** main.cc
** --------------------------------------------------------------------------*/

#include <exception>
#include <iostream>
#include <vector>

#include "reflection.h"

using namespace reflection;

void LvalueReference();
void ReturnValue();
void Functor();
void Lambda();
void ConstLvalueReference();

int main() {
  LvalueReference();
  ReturnValue();
  Functor();
  Lambda();
  ConstLvalueReference();

  return 0;
}

void bar(int x, int &y) {
  y = x;
}

int foo(int x, int y) {
  x = 3;
  return x+y;
}

void LvalueReference() {
  std::vector<Any> args;
  args.emplace_back(1);
  args.emplace_back(10);
  
  auto wrapper_bar = FunctionWrapper(bar);
  wrapper_bar.Call(args);
  
  std::cout << "LvalueReference :" << std::endl;
  for (const auto &x : args)
    std::cout << x.Get<int>() << " ";
  std::cout << std::endl;
  
  std::cout << std::endl;
}

void ReturnValue() {
  std::vector<Any> args;
  args.emplace_back(1);
  args.emplace_back(10);
  
  auto wrapper_foo = FunctionWrapper(foo);
  auto value = wrapper_foo.Call(args);
  
  // call with original call signature
  auto x = wrapper_foo(1, 1);
  
  std::cout << "ReturnValue :" << std::endl;
  std::cout << value.Get<int>() << std::endl;
  for (const auto &x : args)
    std::cout << x.Get<int>() << " ";
  std::cout << std::endl;
  
  std::cout << std::endl;
}

struct A {
  void operator()(int x, float &y) {
    y = x*2;
  }
  
  int operator()(int x, int y) {
    x = 3;
    return x+y;
  }
};

void Functor() {
  std::vector<Any> args;
  args.emplace_back(1);
  args.emplace_back(10.0f);
  
  A t;
  // bind to void operator()(int, float &)
  auto wrapper1 = FunctionWrapper<int, float &>(t);
  // bind to int operator()(int, int) due to overload resolution rules
  auto wrapper2 = FunctionWrapper<int, float>(t);
  
  std::cout << "Functor :" << std::endl;
  wrapper1.Call(args);
  std::cout << args[0].Get<int>() << " " << args[1].Get<float>();
  std::cout << std::endl;
  
  auto value = wrapper2.Call(args);
  std::cout << value.Get<int>() << std::endl;
  std::cout << args[0].Get<int>() << " " << args[1].Get<float>() << std::endl;
  
  std::cout << std::endl;
}

void Lambda() {
  std::vector<Any> args;
  args.emplace_back(12);
  args.emplace_back(10.0f);
  
  auto wrapper1 = FunctionWrapper([](){return 1;});
  auto wrapper2 = FunctionWrapper<int>([](int x) -> float{return x;});
  
  auto value1 = wrapper1.Call(std::vector<Any>{});
  auto value2 = wrapper2.Call(args);
  
  std::cout << "Lambda :" << std::endl;
  std::cout << value1.Get<int>() << std::endl;
  std::cout << value2.Get<float>() << std::endl;
  
  try {
    std::cout << "Cast from any::Any float to double" << std::endl;
    value2.Get<double>();
    std::cout << "Successed" << std::endl;
  } catch(const std::exception &e) {
    std::cout << e.what() << std::endl;
  }
  
  try {
    std::cout << "Cast from any::Any float to double with nothrow" << std::endl;
    auto value = value2.Get<double>(std::nothrow);
    std::cout << "No throw"  << std::endl;
    std::cout << "Pointer Gotten :" << value << std::endl;
  } catch(const std::exception &e) {
    std::cout << e.what() << std::endl;
  }
  
  std::cout << std::endl;
}

void ConstLvalueReference() {
  std::vector<Any> args;
  args.push_back(1);
  args.push_back(2);
  
  auto wrapper_bar = FunctionWrapper(bar);
  auto wrapper_foo = FunctionWrapper(foo);
  
  const auto &const_args = args;
  
  std::cout << "ConstLvalueReference :" << std::endl;
  
  std::cout << "Call foo(int, int)" << std::endl;
  wrapper_foo.Call(const_args);
  std::cout << "Successed" << std::endl << std::endl;
  
  try {
    std::cout << "Call bar(int, int &)" << std::endl;
    wrapper_bar.Call(const_args);
    std::cout << "Successed" << std::endl;
  } catch(const std::exception &e) {
    std::cout << e.what() << std::endl;
  }
  
  std::cout << std::endl;
}

