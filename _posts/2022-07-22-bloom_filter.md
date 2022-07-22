---
title: 布隆过滤器
date: 2022-07-22
categories: [算法和数据结构]
tags: [hash, 布隆过滤器]     # TAG names should always be lowercase
---

> [布隆过滤器](https://zh.m.wikipedia.org/zh/%E5%B8%83%E9%9A%86%E8%BF%87%E6%BB%A4%E5%99%A8)


**基本定义**: 布隆过滤器(Bloom Filter)是1970年由布隆提出的。它实际上是一个很长的二进制向量和一系列随机映射函数。布隆过滤器可以用于检索一个元素是否在一个集合中。  

**基本原理**:布隆过滤器的原理是，当一个元素被加入集合时，通过K个散列函数将这个元素映射成一个位数组中的K个点，把它们置为1。检索时，我们只要看看这些点是不是都是1就（大约）知道集合中有没有它了：如果这些点有任何一个0，则被检元素一定不在；如果都是1，则被检元素很可能在。这就是布隆过滤器的基本思想。

**优点**:
- 插入和查询都是时间都是常数(O(k))
- 节省很大的空间
- 散列函数相互之间无依赖，可以并行实现
- 不存储元素本身，某些对保密要求非常严格的场合有优势
- 多个相同的布隆过滤器可以直接合并
- 可以表示全集

**缺点**:
- 会有很多假正例，即不在过滤器中的会被误判
- 无法删除元素

**用途**:
- 网页爬虫对海量URL的去重，避免爬取相同的URL地址
- 反垃圾邮件，从数十亿个垃圾邮件列表中判断某邮箱是否垃圾邮箱
- 缓存击穿，将已存在的缓存放到布隆过滤器中，当黑客访问不存在的缓存时迅速返回避免缓存及DB挂掉
- 过滤海量无效访问，宁可错杀一千也不放过一个


**实现**:  
```cpp
#include <string>
#include <stdlib.h>
#include <string.h>

class BlooomFilters{
    public:
        BlooomFilters() = delete;

        // num_of_hash ~= table_length / n * ln2
        BlooomFilters(int table_length, int num_of_hash){
            mod = table_length;
            k = num_of_hash;
            bloom_table = new uint8_t[table_length / 8 + 1];
            memset(bloom_table, 0, table_length / 8 + 1);
        }

        ~BlooomFilters(){
            delete bloom_table;
            bloom_table = nullptr;
        }

        // seed 优先使用 2^n -1 (java 使用31),通过不同的seed达到多个hash函数的目的
        uint32_t bkdr_hash(uint32_t seed, std::string str) {
            int len = str.length();
            uint32_t hash = 0;
            for( int i = 0; i < len; i++) {
                hash = hash * seed + str[i];
            }
            return hash % mod;
        }

        bool hash(std::string str) {
            for( size_t i = 0; i < k; i++ ){
                uint32_t seed = (1 << (i + 3)) - 1;
                uint32_t hash_val = bkdr_hash(seed, str);
                // 将hash值对应的位置1
                bloom_table[hash_val / 8] |= 1 << hash_val % 8;
            }
            return true;
        }

        // 会有假正例 
        bool has_hashed(std::string str){
            uint32_t equal = 1;
            for (size_t i = 0; i < k; i++)
            {
                uint32_t seed = (1 << (i + 3)) - 1;
                uint32_t hash_val = bkdr_hash(seed, str);
                equal = equal && bloom_table[hash_val / 8] & (1 << hash_val % 8);
                // printf("%d %d\n",bloom_table[hash_val / 8], 1 << hash_val %8);
            }
            // printf("################\n");
            return equal;
        }

    private:
        uint8_t* bloom_table;
        uint32_t mod, k;
};

int main(int argc, char const *argv[])
{
    
    std::string s = "hello world";
    std::string a = "123456789";
    std::string b = "abcedefkljg";

    BlooomFilters bf(1008611, 5);

    bf.hash(s);
    bf.hash(a);
    bf.hash(b);

    printf("%d %d %d\n", bf.has_hashed(s), bf.has_hashed(a), bf.has_hashed(b));
    printf("%d %d %d\n", bf.has_hashed("fjlajflajf"), bf.has_hashed("roquroqfa"), bf.has_hashed("ghnnv,anv"));

    return 0;
}

```

