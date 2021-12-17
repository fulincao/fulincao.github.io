#include "ARS408.h"

#include <linux/can.h>

int main(int argc, char const *argv[])
{
    
    can_obj_ars408_h_t t;
    double l = -1, lat = -1;

    // 1611196534 520939 CAN6 0x60b 00 4e a4 01 80 20 01 7f

    uint8_t data[8] = { 0x00, 0x4e, 0xa4, 0x01, 0x80, 0x20, 0x01, 0x7f};

    uint64_t my_data = (uint64_t)22135374534934911;
    printf("%lld\n", *(uint64_t*)data);
    int ret = unpack_message(&t, 0x60b, *(uint64_t*)data, 8, 0);


    ret = decode_can_0x60b_Obj_DistLong(&t, &l);
    ret = decode_can_0x60b_Obj_DistLat(&t, &lat);

    printf("ret: %d\n", ret);

    // printf("dist_long: %lf, dis_lat: %lf, tmp: %lf\n", t.can_0x60b_Obj_1_General.Obj_DistLong, t.can_0x60b_Obj_1_General.Obj_DistLat, val);
    printf("dist_long: %lf, dis_lat: %lf\n", l, lat);
    return 0;
}
