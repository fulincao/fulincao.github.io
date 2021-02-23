#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <regex>
#include <assert.h>
#include <iostream>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include <string>
#include <math.h>

#define SHOW_LOG 0

#define debug(a)  if(SHOW_LOG) cout << #a << " is " << a << endl

using namespace std;
using json = nlohmann::json;


typedef struct can_frame{
    int can_id;
    uint8_t can_data[8];
}can_frame;

typedef struct singal
{
    int start;
    int length;
    float scale;
    float offest;
    float min_val;
    float max_val;
    bool little_order;
    bool is_unsigned;
    char* name;
}singal;


typedef struct dbc_message
{
    string name;
    int can_id;
    vector<singal> singals;
}dbc_message;


extern "C" {

    map<int, dbc_message> dbc; 
    map<string, map<int, string> > val_table;

    double decode(singal s, can_frame t) {
        assert(s.length > 0);
        
        // for motorola deal start_bit
        if(!s.little_order) s.start = 8 * (s.start / 8) + (7 - (s.start % 8));

        uint64_t res = 0;
        int now_len = s.length, len = s.length;
        int now_start = s.start, start = s.start;
        uint8_t buf[8], bit_lengths[8];

        int start_bits = start / 8;
        int end_bits = ( start + len - 1 ) / 8;


        // printf("%d %d %d\n", s.start, s.length, s.little_order);
        // printf("%d %d\n", start_bits, end_bits);
        for (size_t i = start_bits ; i <= end_bits; i++){ 
            // printf("%d *\n", t.can_data[i]);  
            int dt = (i+1)*8-now_start;
            if( dt <= now_len){

                if(! s.little_order) {
                    buf[i] = t.can_data[i] << (8 - dt);
                    buf[i] = buf[i] >> (8 - dt);
                } else{
                    buf[i] = t.can_data[i] >> (8 - dt);
                }
                now_len -= dt;
                now_start += dt;
                bit_lengths[i] = dt;
            }else{
                int now_dt = now_len;
                // printf("\n%d %d ", dt, now_dt);

                if ( ! s.little_order) {
                    buf[i] = (t.can_data[i] << (8 - dt));
                    buf[i] = buf[i] >> (8 - now_dt);
                }else {
                    buf[i] = t.can_data[i] >> ( 8 - dt);

                    buf[i] = buf[i] << (8 - now_dt);
                    buf[i] = buf[i] >> (8 - now_dt);
                }

               
                now_len = 0;
                now_start += now_dt;
                bit_lengths[i] = now_dt;

            }
            // printf("%d &\n", buf[i]);
        }
        // printf("\n");

        for (size_t i = 0 ; i <= end_bits-start_bits; i++){   
            uint8_t b = 0;
            if( !s.little_order){
                // if(i + start_bits + 1 <= end_bits) b = bit_lengths[i + start_bits + 1];
                res = res << bit_lengths[i+start_bits] | buf[i+start_bits];
            }else{
                // if(end_bits-i-1 >= 0) b = bit_lengths[end_bits-i-1];
                res = res << bit_lengths[end_bits-i] | buf[end_bits-i];
            }
            // printf("%d * ", res);
        }
        // printf("res :%d\n", res);
        double real_res = res*1.0;
        if (!s.is_unsigned && (res >> (s.length-1) & 0x1))
        {
            res = ~res;
            res = res << 64 - s.length >> 64-s.length;
            debug(res);
            real_res = res*1.0;
            debug(real_res);
            real_res = (real_res + 1) * -1;
            debug(real_res);
        }
        debug(s.scale);
        debug(s.offest);
        debug(s.max_val);
        debug(s.min_val);
        real_res = real_res * s.scale + s.offest;
        real_res = real_res > s.max_val ? s.max_val : real_res;
        real_res = real_res < s.min_val ? s.min_val : real_res;
        // printf("\n%f\n", real_res);
        return real_res;
}

    void add_dbc(char* dbc_path){
        FILE* fp = fopen(dbc_path, "r");

        char buf[10086];
        int last_bo_id = -1;

        while (fgets(buf, 10085, fp) != NULL){
            string target(buf);
            target.erase(0, target.find_first_not_of(" "));
            target.erase(target.find_last_not_of(" ") + 1);
            if( target.substr(0, 3) == "BO_"){
                regex reg("BO_\\s+(\\d+)\\s+(\\w+):");
                smatch sm;
                regex_search(target, sm, reg);
                if(sm.empty()) continue;
               
                string desc = sm[2];
                long long can_id = atoll(sm[1].str().c_str());
                if(can_id > 0x7ff) can_id -= 0x80000000;    


                if (dbc.find(can_id) != dbc.end())
                {
                    cout << "can_id conflict " << endl;
                }
                dbc_message dm;
                dm.can_id = can_id;
                dm.name = desc;
                dbc[can_id] = dm;
                last_bo_id = can_id;

            }else if (target.substr(0, 3) == "SG_"){
                regex reg("SG_\\s+(\\w+)\\s+:\\s+(\\d+)\\|(\\d+)@(\\d+)(.)\\s+\\((.+?),(.+?)\\)\\s+\\[(.*?)\\|(.*?)\\]\\s+\"(.*?)\"");
                smatch sm;
                regex_search(target, sm, reg);
                if(sm.empty()) continue;
                if( last_bo_id == -1) continue;

                singal s;
                s.name = (char*)malloc(strlen(sm[1].str().c_str()) + 1);
                strcpy(s.name, sm[1].str().c_str());
                s.start = atoi(sm[2].str().c_str());
                s.length = atoi(sm[3].str().c_str());
                s.little_order = atoi(sm[4].str().c_str());
                s.is_unsigned = sm[5].str() == "+" ? true : false;
                s.scale = atof(sm[6].str().c_str());
                s.offest = atof(sm[7].str().c_str());
                s.min_val = atof(sm[8].str().c_str());
                s.max_val = atof(sm[9].str().c_str());
                // sm[9] type
                dbc[last_bo_id].singals.push_back(s);
            }else if( target.substr(0, 4) == "VAL_"){
                regex reg("VAL_\\s+(\\d+)\\s+(\\w+)\\s+(\\d+\\s+\".+\"\\s*)+");
                smatch sm;
                regex_search(target, sm, reg);
                if(sm.empty()) continue;
                int can_id = atoi(sm[1].str().c_str());
                string signal_name = sm[2].str();
                string desc = sm[3].str();
                string buf;
                int desc_sz = desc.size();
                int cnt = 0, val = 0;
                string real_val = "";
                map<int, string> v = val_table[to_string(can_id) + "_" + signal_name];
                for( int i = 0; i < desc_sz; i++) {
                    if( desc[i] == '"') continue;
                    if( desc[i] == ' ') {
                        if(!buf.empty()) {
                            if(cnt & 1) {
                                // cout << val << " " << buf << endl;
                                v[val] = buf;
                            }else{
                                val = atoi(buf.c_str());
                            }
                            buf.clear();
                        }
                        cnt += 1;
                        continue;
                    }
                    buf += desc[i];
                }
                val_table[to_string(can_id) + "_" + signal_name] = v;
                // for( int i = 0; i < sm.size(); i++ ) cout << sm[i] << endl;;
            }
        }
        fclose(fp);
        cout << "add dbc file finish..." << endl;
        // for (auto i : dbc)
        // {
        //     cout << i.first << "  " << dbc[i.first].name << endl;
        //     for (auto s : dbc[i.first].singals)
        //     {
        //        cout << s.name << " ";
        //     }
        //     cout << endl;

        // }
    }

    char* decode_message(can_frame t, bool use_val_table=true){
        json j;
        if(dbc.find(t.can_id) == dbc.end()){
            string str = j.dump();
            char *cstr = new char[str.length() + 1];
            strcpy(cstr, str.c_str());

            return cstr;
        } 
        for( auto s: dbc[t.can_id].singals) {
          
            string tmp(s.name);
            double v = decode(s, t);

            if( use_val_table) {
                string key = to_string(t.can_id) + "_" + string(s.name);
                if(val_table.find(key) != val_table.end()) {
                    int64_t int_v = (int64_t)v;
                    string real_val = val_table[key][int_v];
                    cout << s.name << " " << real_val << endl;
                    j[s.name] = real_val;
                    continue;
                }
            }

            // 精度范围内,自动调整类型. 避免解析signal数据类型
            int64_t int_v = v;
            j[s.name] = v;
            if( fabs(v - int_v) < 1e-5) j[s.name] = int_v;
            // cout << s.name << " " << v << endl;
        }
        // 方便python调用,这里返回的jsn字符串.
        string str = j.dump();
        char *cstr = new char[str.length() + 1];
        strcpy(cstr, str.c_str());

        return cstr;
    }

}

// ars @0
void test_ars(){
    add_dbc("/home/cao/work-git/cve/cve/dbc/ARS408.dbc");
    
    FILE* fp = fopen("./test_can_parser_data/ars.txt", "r");

    char buf[10086];
    uint32_t sec, us;
    int can_id, can_data[8];
    char tempc[256];
    can_frame test;
    while (fgets(buf, 10085, fp) != NULL){
        string target(buf);

        sscanf(buf, "%u %u %s %x %x %x %x %x %x %x %x %x", &sec, &us,
                                            tempc, &can_id, can_data, can_data+1,
                                            can_data+2, can_data+3, can_data+4, can_data+5,
                                            can_data+6, can_data+7);

        test.can_id = can_id;
        for( int i = 0; i < 8; i++ ) test.can_data[i] = can_data[i];
        char* t = decode_message(test, false);
        printf("%s\n", t);
    }
}

// @1
void test_x1j(){
     add_dbc("/home/cao/work-git/cve/cve/dbc/X1_AEB_20200812.dbc");
    
    FILE* fp = fopen("./test_can_parser_data/x1j.txt", "r");

    char buf[10086];
    uint32_t sec, us;
    int can_id, can_data[8];
    char tempc[256];
    can_frame test;
    while (fgets(buf, 10085, fp) != NULL){
        string target(buf);

        sscanf(buf, "%u %u %s %x %x %x %x %x %x %x %x %x", &sec, &us,
                                            tempc, &can_id, can_data, can_data+1,
                                            can_data+2, can_data+3, can_data+4, can_data+5,
                                            can_data+6, can_data+7);

        test.can_id = can_id;
        for( int i = 0; i < 8; i++ ) test.can_data[i] = can_data[i];
        char* t = decode_message(test, false);
        printf("%s\n", t);
        // break;
    }
}


int main(int argc, char* argv[]){

    dbc.clear();
    val_table.clear();
    test_ars();
    
    dbc.clear();
    val_table.clear();
    test_x1j();
    return 0;
}
