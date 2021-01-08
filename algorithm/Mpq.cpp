
#include <string>

class Mpq{

    public:

        struct hash_table
        {
            unsigned long hash_a = -1;
            unsigned long hash_b = -1;
        };

        Mpq() = delete;
        Mpq(int hash_size){
            mod = hash_size;
            table = new hash_table[mod];

            init_crypt_table();
        }

        ~Mpq(){
            delete table;
            table = nullptr;
        }

        void init_crypt_table(){
            unsigned long seed = 0x00100001, index1 = 0, index2 = 0, i;
            for( index1 = 0; index1 < 0x100; index1++) {
                // 按列初始化
                for (index2 = index1, i = 0; i < 5; index2+=0x100, i++){
                    unsigned long temp1, temp2;
                    seed = (seed*125 + 3) % 0x2AAAAB;
                    temp1 = (seed & 0xFFFF) << 0x10;
                    seed = (seed*125 + 3) % 0x2AAAAB;
                    temp2 = seed & 0xFFFF;

                    crypt_table[index2] = temp2 | temp1;
                }
                

            }
        }

        unsigned long inner_hash_str(std::string str, unsigned long down_type=0){
            int len = str.length();
            unsigned long seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
            
            for (size_t i = 0; i < len; i++)
            {
                int ch = str[i];
                seed1 = crypt_table[(down_type << 8 ) + ch] ^ (seed1 + seed2);
                seed2 = ch + seed2 + seed1 + (seed2 << 5) + 3;
            }
            return seed1;
        }

        bool hash(std::string str) {
            unsigned long hash_offset = 0, hash_oa = 1, hash_ob = 2;
            unsigned long hash_s = inner_hash_str(str, hash_offset) % mod;
            unsigned long hash_a = inner_hash_str(str, hash_oa);
            unsigned long hash_b = inner_hash_str(str, hash_ob);
            unsigned long hash_pos = hash_s;

            // 冲突往后移, 非挂链表形式
            while( table[hash_pos].hash_a != -1) {
                hash_pos = (hash_pos + 1) % mod;
                // 找了一圈没有找到位置,就满了(极限情况下,退化成O(n))
                if( hash_pos == hash_s) return false;
            }
            table[hash_pos].hash_a = hash_a;
            table[hash_pos].hash_b = hash_b;
            return true;
        }
        
        int64_t hashed(std::string str) {
            unsigned long hash_offset = 0, hash_oa = 1, hash_ob = 2;
            unsigned long hash_s = inner_hash_str(str, hash_offset) % mod;
            unsigned long hash_a = inner_hash_str(str, hash_oa);
            unsigned long hash_b = inner_hash_str(str, hash_ob);
            unsigned long hash_pos = hash_s;

            while (table[hash_pos].hash_a != -1)
            {
                if( table[hash_pos].hash_a == hash_a && table[hash_pos].hash_b == hash_b) return hash_pos;
                hash_pos = (hash_pos + 1) % mod;
                if( hash_pos == hash_s) break;
            }
            return -1;
        }


    private:
        unsigned long crypt_table[0x500];
        unsigned long mod;
        hash_table* table;

};



int main(int argc, char const *argv[])
{
    
    std::string s = "hello world";
    std::string a = "123456789";
    std::string b = "abcedefkljg";
    Mpq mpq(1008611);
    mpq.hash(s);
    mpq.hash(a);
    mpq.hash(b);
    printf("%lld %lld %lld\n", mpq.hashed(s), mpq.hashed(a), mpq.hashed(b));
    return 0;
}

