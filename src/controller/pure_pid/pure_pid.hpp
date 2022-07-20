#include <string.h>

class PurePid
{
private:
    double errs[3];
    int err_cnt;
public:
    double _kp, _ki, _kd;

    PurePid(double kp, double ki, double kd);
    double step_control(double val, double err);
};

