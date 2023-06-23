# **akr::IntrusiveList**

## **Contents**

  - [1. Require](#1-require)
  - [2. Usage](#2-usage)

## **1. Require**
* ### `C++20`

## **2. Usage**
```c++
#include "intrusivelist.hh"

struct Test: akr::IntrusiveNode<Test>
{
    int value {};

    Test(int value_) noexcept:
        value { value_ }
    {
    }
};

akr::IntrusiveList<Test> list;

auto v1 = Test(1);
auto v2 = Test(2);
auto v3 = Test(3);

list.InsertLast(&v1);
list.InsertLast(&v2);
list.InsertLast(&v3);

std::printf("%zu\n", list.GetLength());

for (auto&& e : list)
{
    std::printf("%d ", e.value);
}
std::puts("");

list.Remove(&v2);

std::printf("%zu\n", list.GetLength());

for (auto&& e : list)
{
    std::printf("%d ", e.value);
}
std::puts("");
```
