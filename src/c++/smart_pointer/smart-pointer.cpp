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
