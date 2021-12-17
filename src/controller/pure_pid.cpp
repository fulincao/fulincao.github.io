#include "pure_pid.hpp"

PurePid::PurePid(double kp, double ki, double kd)
{
    _kd = kd;
    _kp = kp;
    _ki = ki;
    err_cnt = 0;
    memset(errs, 0, sizeof(errs));
}

double PurePid::step_control(double val, double err) {
    double res = val;
    errs[err_cnt%3] = err;
    
    if(err_cnt >= 3) 
    {
        res += _kp * (errs[err_cnt % 3] - errs[(err_cnt-1) % 3]) + _ki * errs[err_cnt % 3] + _kd * (errs[err_cnt % 3] - 2 * errs[(err_cnt - 1) % 3] + errs[(err_cnt - 2) % 3]);
    }
    err_cnt++;
    return res;
}