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

        // seed 优先使用 2^n -1 (java 使用31)
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
                bloom_table[hash_val / 8] |= 1 << hash_val % 8;
                // printf("%d\n", bloom_table[hash_val/8]);
            }
            // printf("---------\n");
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

