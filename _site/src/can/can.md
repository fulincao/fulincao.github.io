# 从使用端分享下can解析

从个人使用角度以及工作所接触了解的角度分享一下对can的了解, 有什么不对的请多见谅.

## 数据

我们一般拿到的can数据一般都是这种样子的

|  CAN ID   | CAN DATA  |
|  ----  | ----  |
| 0x60b  | 0x11 0x22 0x33 0x44 0x55 0x66 0x77 0x88 |

这便是一条完整的can帧, 由id和data组成.其在linux中结构定义如下:

```c++
#include <linux/can.h>

/* CAN payload length and DLC definitions according to ISO 11898-1 */
#define CAN_MAX_DLC 8
#define CAN_MAX_DLEN 8


/* special address description flags for the CAN_ID */
#define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG 0x20000000U /* error message frame */


struct can_frame {
    canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    __u8    can_dlc; /* frame payload length in byte (0 .. CAN_MAX_DLEN) */
    __u8    __pad;   /* padding */
    __u8    __res0;  /* reserved / padding */
    __u8    __res1;  /* reserved / padding */
    __u8    data[CAN_MAX_DLEN] __attribute__((aligned(8)));
};

```

can帧的主要几类:

- **数据帧-标准帧**: 长度11位,最大值为0x7ff.

- **数据帧-扩展帧**: 长度29位, 可通过canid & CAN_EFF_FLAG == 1 判断.

- **错误帧**: 通过 CAN_ERR_FLAG 判断
- **远程帧**: 通过 CAN_RTR_FLAG 判断

其中我们看到最多的是数据帧, 其他帧基本没有看到过.

## dbc

有了数据之后,如何解码出我们所需要的信息呢? 通常情况都是通过dbc来编码或者解码的.DBC是Database Can的缩写，其代表的是CAN的数据库文件，在这个文件中把CAN通讯的信息定义完整.

```text
BO_ 1547 Obj_1_General: 8 ARS_ISF
 SG_ Obj_DynProp : 50|3@0+ (1,0) [0|7] ""  ExternalUnit
 SG_ Obj_RCS : 63|8@0+ (0.5,-64) [-64|63.5] "dBm²"  ExternalUnit
 SG_ Obj_VrelLat : 45|9@0+ (0.25,-64) [-64|63.75] "m/s"  ExternalUnit
 SG_ Obj_ID : 7|8@0+ (1,0) [0|255] ""  ExternalUnit
 SG_ Obj_DistLong : 15|13@0+ (0.2,-500) [-500|1138.2] "m"  ExternalUnit
 SG_ Obj_VrelLong : 39|10@0+ (0.25,-128) [-128|127.75] "m/s"  ExternalUnit
 SG_ Obj_DistLat : 18|11@0+ (-0.2,204.6) [-204.8|204.6] "m"  ExternalUnit

```

上面展示了一段ars的dbc中对canid为1547的can帧描述.其中主要有**BO_** 报文, **SG_** 信号.

一个报文下会有多个信号. 一个报文就是一个can帧,其数据域长度为8字节64位(见前面定义).对于信号,每一行都相信的描述了该信号在数据域中的位置,解码方式等.

### BO_ (报文)

基本格式如下:
**BO_ MessageId MessageName: MessageSize Transmitter**

1. BO_为关键字，表示报文；
2. MessageId为定义的报文ID，是以10进制数表示；
3. MessageName表示该报文的名字
4. MessageSize表示该报文数据域字节数，为无符号整型数据；
5. Transmitter表示发送该报文的网络节点；如果该报文没有指定发送节点，则该值需设置为” Vector__XXX”或者不写

以1547报文举例说明:
**BO_ 1547 Obj_1_General: 8 ARS_ISF**

| 定义| 描述|
|  ----  | ----  |
| BO_ | 报文关键字 |
| 1547| can id 16进制为0x60b|
| Obj_1_General | 报文名字 |
|8 | 报文数据域字节数|
|ARS_ISF | 发送该报文的节点 |

### SG_ (信号)

基本格式如下:
**SG_ SignalName : StartBit|SignalSize@ByteOrder ValueType (Factor,Offset) [Min|Max] Unit Receiver**

1. SG_为关键字，表示信号；
2. SignalName、 StartBit、 SignalSize分别表示该信号的名字、起始位、信号长度；
3. ByteOrder表示信号的字节顺序：0代表Motorola格式(大端序)，1代表Intel格式(小端序)；
4. ValueType 表示该信号的数值类型：+表示无符号数，-表示有符号数；
5. Factor表示因子，Offset表示偏移量；这两个值于该信号的原始值与物理值之间的转换。转换如下：物理值=原始值*因子+偏移量；
6. Min|Max表示该信号的最小值和最大值，即指定了该信号值的范围；这两个值为double类型；
7. Unit表示该信号的单位，为字符串类型；
8. Receiver表示该信号的接收节点；若该信号没有指定的接收节点，则必须设置为” Vector__XXX”

举例如下:
**SG_ Obj_DistLat : 18|11@0+ (-0.2,204.6) [-204.8|204.6] "m"  ExternalUnit**
| 定义| 描述|
|  ----  | ----  |
| SG_ | 信号关键字 |
| Obj_DistLat| 信号名|
| 18| 起始位 |
| 11| 长度|
|0 | motorola格式(大端序) |
| +| 无符号数 |
| -0.2| 缩放因子|
| 204.6| 偏移量 |
| -204.8| 最小值|
| 204.6| 最大值 |
| m| 单位|
|ExternalUnit| 接收节点|

## 解析

通过dbc或者协议文档获取了报文和信号的编解码信息,然后就可以解析出明文.这里还是以conti的ars 408雷达举例.

### 大端序列

cve采集到的一条ars报文:
**0x60b 00 4e a4 01 80 20 01 7f**

- 首先构建位图

    将8字节64位数据转成二进制,依次展开
    **大端序列从左往右编号, 小端序从右往左编号**, 从上到下依次增加
    |位序编号| 0|1 | 2|3 | 4|5 | 6|7|
    |----|  ----  | ----  |  ----  | ----  |  ----  | ----  |  ----  | ----  |  ----  | ----  |----|
    |**0x00**|0 |0 |0 |0 |0 |0 |0 |0 |
    |**0x4e**|0 |1 |0 |0 |1 |1 |1 |0 |
    |**0xa4**|1 |0 |1 |0 |0 |1 |0 |0 |
    |**0x01**|0 |0 |0 |0 |0 |0 |0 |1 |
    |**0x80**|1 |0 |0 |0 |0 |0 |0 |0 |
    |**0x20**|0 |0 |1 |0 |0 |0 |0 |0 |
    |**0x01**|0 |0 |0 |0 |0 |0 |0 |1 |
    |**0x7f**|0 |1 |1 |1 |1 |1 |1 |1 |

- 根据信号描述获取对应字节
  这里以Obj_DistLat信号作为示例.如上所示,在dbc中定义的起始位start_bit = 18, 长度length = 11.
  **大端序的实际起始位需要转换, 小端序无需转换**,转换如下:

  ```cpp
  start_bit = 8 * (start_bit / 8) + (7 - (start_bit % 8));
  ```

  转换之后为 stat_bit = 21, length = 11,
  - 然后从位图中找到这11位是 0xa4的后3位 加上 0x01的8位即:100 00000001

  - 然后按照大端序排列为 10000000001, 即10进制为val = 1025;
  - 由于当前信号是无符号数据,所以补码等于源码,如果是有符号数且是负数的话需要取反加1计算(后面会介绍)
  - 缩放 scale = -0.2, offset = 204.6, val = val * -0.2 + 204.6 = -0.4
  - 即算得 Obj_DistLat在当前帧的值为-0.4, 其他信号按此步骤计算可得
  
  ```json
  {
      "Obj_DistLong": 3.2000000000000455, 
      "Obj_VrelLong": 0.0, 
      "Obj_DynProp": 1, 
      "Obj_ID": 0, 
      "Obj_RCS": -0.5, 
      "Obj_VrelLat": 0.0, 
      "Obj_DistLat": -0.4000000000000057
  }
  ```

  至此当前帧解析完毕.

### 小端序列

cve采集到的一条x1j报文:
**0x76d 00 00 00 00 ae 1f 00 00**

其0x76d报文定义如下:

```text

BO_ 1901 KeyCarFrameA1: 8 MINIEYE_TRANSMITTER
 SG_ on_route : 1|1@1+ (1,0) [0|1] "" Vector__XXX
 SG_ TargetVehicle_Status : 20|4@1+ (1,0) [0|15] "" Vector__XXX
 SG_ TargetVehicle_Width : 24|8@1+ (0.05,0) [0|12.5] "M" Vector__XXX
 SG_ FCW : 0|1@1+ (1,0) [0|1] ""  ADAS
 SG_ Vehicle_ID : 2|6@1+ (1,0) [0|63] ""  ADAS
 SG_ TargetVehicle_PosX : 8|12@1+ (0.0625,0) [0|250] "m"  ADAS
 SG_ TargetVehicle_PosY : 32|10@1- (0.0625,0) [-31.9375|31.9375] "m"  ADAS
 SG_ TargetVehicle_Type : 48|3@1+ (1,0) [0|7] ""  ADAS
```

这里考虑TargetVehicle_PosY, 由上面分析可得 start_bit = 32, length = 10, scale = 0.0625, offset = 0,小端序, 有符号.

- 构建位图
    此时**位序与大端序号刚刚相反**
    |位序编号| 7|6| 5|4| 3|2| 1|0|
    |----|  ----  | ----  |  ----  | ----  |  ----  | ----  |  ----  | ----  |  ----  | ----  |----|
    |**0x00**|0 |0 |0 |0 |0 |0 |0 |0 |
    |**0x00**|0 |0 |0 |0 |0 |0 |0 |0 |
    |**0x00**|0 |0 |0 |0 |0 |0 |0 |0 |
    |**0x00**|0 |0 |0 |0 |0 |0 |0 |0 |
    |**0xae**|1 |0 |1 |0 |1 |1 |1 |0 |
    |**0x1f**|0 |0 |0 |1 |1 |1 |1 |1 |
    |**0x00**|0 |0 |0 |0 |0 |0 |0 |0 |
    |**0x00**|0 |0 |0 |0 |0 |0 |0 |0 |

- 然后从位图中找到这10位是 0xae 加上 0xff的后两位 即:10101110 11
- 然后按照小端序排列为 1110101110, 即10进制为val = 942;
- 由于当前信号是有符号数据,且最高位为1,则说明该值为负数,需要取补码,补码为原码取反加1,则:val = ((~val) + 1) *-1 = -82
- 缩放加偏差为val = val * 0.0625 + 0 = -5.125
- 即算得 TargetVehicle_PosY在当前帧的值为-5.125, 其他信号按此步骤计算可得
  
  ```json
    {
        "TargetVehicle_PosX": 0.0, 
        "TargetVehicle_Status": 0, 
        "TargetVehicle_Type": 0, 
        "FCW": 0, 
        "on_route": 0, 
        "TargetVehicle_PosY": -5.125, 
        "Vehicle_ID": 0, 
        "TargetVehicle_Width": 0.0
    }
  ```

### 实现

```cpp

// 定义数据结构
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

map<int, dbc_message> dbc; 
map<string, map<int, string> > val_table;

// 加载dbc
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
    }

// 解码信号
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

```

完整代码见can_parser.cpp, 验证脚本见test_can.py.本人测试cve采集的一组x1j和ars数据时在1e-5的精度下能保证100%准确率. 由于未做大量验证,无法保证完全没有问题.

### 使用第三方库

#### python - cantools

```python

import cantools
dbc = cantools.database.load_file("/home/cao/work-git/cve/cve/dbc/ARS408.dbc", strict=False)

rf = open("./test_can_parser_data/ars.txt", "r")
ids = [m.frame_id for m in dbc.messages]

for line in rf:
    cols = line.split()
    can_id = int(cols[3], 16)
    data = b''.join([int(x, 16).to_bytes(1, 'little') for x in cols[4:]])

    if can_id in ids:
        j = dbc.decode_message(can_id, data, decode_choices=False)
        print(json.dumps(j))
        break

```

#### cpp - dbcc

- github链接 : <https://github.com/howerj/dbcc>
- 安装完之后 dbcc ARS408.dbc 便会生成dbc对应的.h 和.c.
- 使用
  
  ``` cpp

    #include "ARS408.h"
    #include <linux/can.h>

    int main(int argc, char const *argv[])
    {
        can_obj_ars408_h_t t;
        double l = -1, lat = -1;

        // 1611196534 520939 CAN6 0x60b 00 4e a4 01 80 20 01 7f

        uint8_t data[8] = { 0x00, 0x4e, 0xa4, 0x01, 0x80, 0x20, 0x01, 0x7f};

        printf("%lld\n", *(uint64_t*)data);
        int ret = unpack_message(&t, 0x60b, *(uint64_t*)data, 8, 0);


        ret = decode_can_0x60b_Obj_DistLong(&t, &l);
        ret = decode_can_0x60b_Obj_DistLat(&t, &lat);

        printf("ret: %d\n", ret);

        // printf("dist_long: %lf, dis_lat: %lf, tmp: %lf\n", t.can_0x60b_Obj_1_General.Obj_DistLong, t.can_0x60b_Obj_1_General.Obj_DistLat, val);
        printf("dist_long: %lf, dis_lat: %lf\n", l, lat);
        return 0;
    }

  ```
