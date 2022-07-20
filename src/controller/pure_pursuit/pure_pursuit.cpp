#include <math.h>

#include "pure_pursuit.hpp"

double PurePursuit::cal_ld(double vel){
    return 1.0 / ( 2 * _max_braking_acc) * vel * vel + _reactime * vel + _min_turning_radius;
}

double PurePursuit::cal_gps_distance(double lng1, double lat1, double lng2, double lat2){
    double radius_of_earth = 6378137.0;
    lng1 = lng1 / 180 * M_PI;
    lat1 = lat1 / 180 * M_PI;
    lng2 = lng2 / 180 * M_PI;
    lat2 = lat2 / 180 * M_PI;

    double delt_lat = lat2 - lat1;
    double delt_lng = lng2 - lng1;
    double a = pow(sin(0.5 * delt_lat), 2) + pow(sin(0.5 * delt_lng), 2) * cos(lat1) * cos(lat2);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return radius_of_earth * c;
}

 double PurePursuit::cal_gps_bearing(double lng1, double lat1, double lng2, double lat2){
    lng1 = lng1 / 180 * M_PI;
    lat1 = lat1 / 180 * M_PI;
    lng2 = lng2 / 180 * M_PI;
    lat2 = lat2 / 180 * M_PI;

    double delt_lat = lat2 - lat1;
    double delt_lng = lng2 - lng1;
    double y = sin(delt_lng) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(delt_lng);
    return atan2(y, x);
 }

 double PurePursuit::cal_lateral(double lng1, double lat1, double lng2, double lat2, double yaw){
    double distance = cal_gps_distance(lng1, lat1, lng2, lat2);
    double heading = cal_gps_bearing(lng1, lat1, lng2, lat2);
    if (heading < 0) heading += 2 * M_PI;
    heading -= yaw;
    double x = sin(heading) * distance;
    double y = cos(heading) * distance;
    return y;
 }

double PurePursuit::cal_wheel_angle(double lng1, double lat1, double lng2, double lat2, double vel, double yaw){
    double ld = cal_ld(vel);
    double lateral = cal_lateral(lng1, lat1, lng2, lat2, 0);
    return atan(2 * _wheel_base * lateral / ld / ld);
}

void PurePursuit::run(double lng, double lat, double alt){
    ;
}
