---
title: 智能指针
date: 2022-02-24
categories: [C++编程]
tags: [智能指针]     # TAG names should always be lowercase
---

## RAII与引用计数
>  [现代c++教程](https://changkun.de/modern-cpp/zh-cn/05-pointers/)

在传统 C++ 中，需要手动释放资源，有可能就忘记了去释放资源而导致泄露。所以通常的做法是对于一个对象而言，**在构造函数的时候申请空间，而在析构函数（在离开作用域时调用）的时候释放空间，也就是我们常说的 RAII 资源获取即初始化技术**。

凡事都有例外，我们总会有需要将对象在自由存储上分配的需求，在传统 C++ 里我们只好使用 new 和 delete 去 『记得』对资源进行释放。而 C++11 引入了智能指针的概念，使用了引用计数的想法，让程序员不再需要关心手动释放内存。 这些智能指针就包括 **std::shared_ptr、std::unique_ptr、std::weak_ptr**，使用它们需要包含头文件 <memory>。

## std::shared_ptr
**std::shared_ptr**是一种智能指针，它能够记录多少个**shared_ptr**共同指向一个对象，从而消除显式的调用**delete**，当引用计数变为零的时候就会将对象自动删除。但还不够，因为使用**std::shared_ptr**仍然需要使用**new**来调用，这使得代码出现了某种程度上的不对称,**std::make_shared**就能够用来消除显式的使用 new，所以**std::make_shared**会分配创建传入参数中的对象，并返回这个对象类型的**std::shared_ptr**指针.  

```cpp
#include <iostream>
#include <memory>

int main() {
    // auto pointer = new int(10); // illegal, no direct assignment
    // Constructed a std::shared_ptr
    std::shared_ptr<int> pointer = std::make_shared<int>(10);
    std::cout << *pointer << std::endl; // 11
    return 0;
}
```

**std::shared_ptr**可以通过**get()** 方法来获取原始指针，通过**reset()** 来减少一个引用计数，并通过**use_count()** 来查看一个对象的引用计数。例如：
```cpp

auto pointer = std::make_shared<int>(10);
auto pointer2 = pointer; // 引用计数+1
auto pointer3 = pointer; // 引用计数+1
int *p = pointer.get(); // 这样不会增加引用计数
std::cout << "pointer.use_count() = " << pointer.use_count() << std::endl; // 3
std::cout << "pointer2.use_count() = " << pointer2.use_count() << std::endl; // 3
std::cout << "pointer3.use_count() = " << pointer3.use_count() << std::endl; // 3

pointer2.reset();
std::cout << "reset pointer2:" << std::endl;
std::cout << "pointer.use_count() = " << pointer.use_count() << std::endl; // 2
std::cout << "pointer2.use_count() = " << pointer2.use_count() << std::endl; // 0, pointer2 已 reset
std::cout << "pointer3.use_count() = " << pointer3.use_count() << std::endl; // 2
pointer3.reset();
std::cout << "reset pointer3:" << std::endl;
std::cout << "pointer.use_count() = " << pointer.use_count() << std::endl; // 1
std::cout << "pointer2.use_count() = " << pointer2.use_count() << std::endl; // 0
std::cout << "pointer3.use_count() = " << pointer3.use_count() << std::endl; // 0, pointer3 已 reset
```

## std::unique_ptr

**std::unique_ptr**是一种独占的智能指针，它禁止其他智能指针与其共享同一个对象，从而保证代码的安全：
```cpp
std::unique_ptr<int> pointer = std::make_unique<int>(10); // make_unique 从 C++14 引入
std::unique_ptr<int> pointer2 = pointer; // 非法
```

## std::weak_ptr
**std::weak_ptr**是一种弱引用（相比较而言**std::shared_ptr**就是一种强引用）。弱引用不会引起引用计数增加.**std::weak_ptr**没有 * 运算符和 -> 运算符，所以不能够对资源进行操作，它可以用于检查**std::shared_ptr**是否存在，其expired()方法能在资源未被释放时，会返回 false，否则返回 true；除此之外，它也可以用于获取指向原始对象的 std::shared_ptr 指针，其 lock() 方法在原始对象未被释放时，返回一个指向原始对象的 std::shared_ptr 指针，进而访问原始对象的资源，否则返回nullptr。


## 代码实现
一种简单的shared_ptr实现

```cpp
#include <string.h>

#include <map>
#include <memory>
#include <string>
#include <string>
#include <iostream>

std::map<uint64_t, uint64_t> ref_count;

template <typename T>
class MySharedPointer
{
private:
    T *value_;
    uint64_t idx_;

public:
    MySharedPointer(T &value)
    {
        value_ = new T;

        memcpy(value_, &value, sizeof(T));
        idx_ = reinterpret_cast<uint64_t>(value_);
        ref_count[idx_] += 1;
    }
    ~MySharedPointer()
    {
        std::cout << *value_ << std::endl;
        uint64_t count = ref_count[idx_];
        if (count > 0)
        {
            count -= 1;
            ref_count[idx_] -= 1;
        }
        if (count <= 0)
        {
            printf("destory memory ..\n");
            delete value_;
            ref_count.erase(idx_);
        }
    }

    MySharedPointer(MySharedPointer &others)
    {
        idx_ = others.idx_;
        value_ = others.value_;
        ref_count[idx_] += 1;
    }
};

struct TestNode
{
    int a, b;
    double c;
    friend std::ostream & operator<< (std::ostream &os, TestNode &c)
    {
        os << c.a << "#" << c.b << "#" << c.c ;
        return os;
    }
};

int main(int argc, char *argv[])
{

    TestNode node;
    node.a = node.b = node.c = 10;
    MySharedPointer<TestNode> a(node);
    MySharedPointer<TestNode> b(a);
    return 0;
}


```