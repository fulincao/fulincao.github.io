#include<string>


class PurePursuit {

    public:
        PurePursuit(double max_braking_acc, double reactime, double min_turning_radius, double wheel_base):_max_braking_acc(max_braking_acc),
            _reactime(reactime), _min_turning_radius(min_turning_radius), _wheel_base(wheel_base){
        }
        void load_global_path(std::string log_path);
        double cal_ld(double vel);
        double cal_gps_distance(double lng1, double lat1, double lng2, double lat2);
        double cal_gps_bearing(double lng1, double lat1, double lng2, double lat2);
        double cal_lateral(double lng1, double lat1, double lng2, double lat2, double yaw=0);
        double cal_wheel_angle(double lng1, double lat1, double lng2, double lat2, double vel, double yaw=0);
        void run(double lng, double lat, double alt);

    private:
        double _max_braking_acc, _reactime, _min_turning_radius;
        double _wheel_base;

};