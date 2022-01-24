from math import atan, cos, sqrt
from os import path
from math import pi, sin, cos
import math
import numpy as np
import matplotlib.pyplot as plt
import scipy.linalg as la

from spline import Spline2D
from dynamics_module import *
from lqr import LQR

def generator_path(x_base, y_base):
    """[summary]
        use spline 2d get path
    """
    sp2d = Spline2D(x_base, y_base)
    
    res = []
    for i in np.arange(0, sp2d.s[-1], 0.1):
        x, y = sp2d.calc_position(i)
        yaw = sp2d.calc_yaw(i)
        curvature = sp2d.calc_curvature(i)
        res.append([x, y, yaw, curvature])

    return res

def set_AB(vx, dt):
    """[summary]
    Args:
        A，B :  X = Ax + Bu
        vx ([type]): [车辆x方向速度]

    --------
        0   1                                  0                           0
    A = 0   -(cf + cr) / (m*vx)                (cf + cr) / m               (-cf * lf + cr * lr) / (m * vx)
        0   0                                  0                           1
        0   -(cf * lf - cr * lr) / (Iz * vx)   (cf * lf - cr * lr) / Iz    -(cf *lf^2 + cr * lr^2) / (Iz * vx)
    --------
        0  
        cf / m
    B = 0
        (cf * lf) / Iz
    --------
    """

    A = np.zeros((4, 4), dtype=np.float64)
    B = np.zeros((4, 1), dtype=np.float64)

    A[0][1] = 1
    
    A[1][1] = -(Cf + Cr) / (M * vx)
    A[1][2] = (Cf + Cr) / M
    A[1][3] = (-Cf * Lf + Cr * Lr) / (M * vx)

    A[2][3] = 1
    
    A[3][1] = -(Cf * Lf - Cr * Lr) / (Iz * vx)
    A[3][2] = (Cf * Lf - Cr * Lr) / Iz
    A[3][3] = -(Cf * Lf * Lf + Cr * Lr * Lr) / ( Iz * vx)


    B[1][0] = Cf / M
    B[3][0] = Cf * Lf / Iz

    """
    离散变化 https://baike.baidu.com/item/%E5%8F%8C%E7%BA%BF%E6%80%A7%E5%8F%98%E6%8D%A2/19123826?fr=aladdin
    Ad = (1 + A*T/2) / (1 - A*T/2)
    """
    I = np.eye(4, dtype=np.float64)
    Ad = (I + A * dt / 2) @ la.inv( I - A* dt / 2 )
    Bd = B * dt


    Q = np.eye(4, dtype=np.float64)
    R = np.eye(1, dtype=np.float64)

    Q[0][0] = 5
    R[0][0] = 50
    return Ad, Bd, Q, R

def cal_feed_forward_angle(vx, curvature, k3):
    """[summary]
    Args:
        vx : [x方向的速度]
        curvature ([type]): [道路曲率 = 1/R ]
        k3 ([type]): [lqr k 矩阵第三个值]
    """
    
    # 不足转向系数
    kv = (Lr * M) / (Cf * (Lf + Lr) ) - (Lf * M) / (Cr * (Lf + Lr))

    # ay 向心加速度 
    ay = vx * vx * curvature

    forward_angle = L * curvature + kv * ay - k3 * (Lr *curvature - Lf * M * vx * vx * curvature / Cr / L)
    return forward_angle

def cal_feed_back_angle(k, x):
    """[summary]

    Args:
        k ([type]): [lqr k gain]
        x ([type]): [state]

    Returns:
        [type]: [feedback angle]
    """
    return (-k * x)[0][0]


def run():
    # base path
    x = [0.0, 6.0, 12.5, 10.0, 7.5, 3.0, -1.0]
    y = [0.0, -3.0, -5.0, 6.5, 3.0, 5.0, -2.0]

    # init plan path
    plan_path = generator_path(x, y)
    plan_x = [x[0] for x in plan_path]
    plan_y = [x[1] for x in plan_path]
    plan_yaw = [x[2] for x in plan_path]
    plan_crt = [x[3] for x in plan_path]

    plt.plot(plan_x, plan_y)

    # init car model
    init_vx = 10 / 3.6
    init_idx = 0
    dm = DynamicsModel(x=plan_path[init_idx][0]-1 , y=plan_path[init_idx][1]-1, vx=init_vx)
    
    # init lqr state    
    X = np.zeros((4, 1), dtype=np.float64)
    lqr = LQR()

    # track parm
    target_pts_idx = 0
    iter = 0
    dt = 0.1

    last_lateral_error = 0
    last_heading_error = 0
    while iter < 200:
        if target_pts_idx >= len(plan_x):
            break

        iter += 1
        dx = dm.x- plan_x[target_pts_idx]
        dy = dm.y - plan_y[target_pts_idx]
        euler_dist = math.sqrt(dx**2 + dy**2)
    
        # cos_target_heading = cos(pi / 2 - plan_yaw[target_pts_idx])
        # sin_target_heading = sin(pi / 2 - plan_yaw[target_pts_idx])
        # lateral_error = cos_target_heading * dx - sin_target_heading * dy 
        # longtidual_error = sin_target_heading * dx + cos_target_heading * dy
        psi_des = plan_yaw[target_pts_idx]
        lateral_error = -dy * cos(psi_des) + dx * sin(psi_des)
        longtidual_error = dy * sin(psi_des) + dx * cos(psi_des)
        heading_error = dm.yaw - psi_des
        print("iter:", iter, ", target_pts:", target_pts_idx)
        print("lateral_error, longtidual_error:", lateral_error, longtidual_error)
        
        if longtidual_error >= 0:
            target_pts_idx += 1
            continue

        Ad, Bd, Q, R = set_AB(dm.vx, dt)
        # X[0][0] = lateral_error
        # X[1][0] = dm.vx * (dm.yaw - psi_des) + dm.vy
        # X[2][0] = dm.yaw - psi_des
        # X[3][0] = angular_v - target_vx * target_curvarate 其他仪器测量得到车辆角速度

        X[0][0] = lateral_error
        X[1][0] = (lateral_error - last_lateral_error) / dt
        X[2][0] = heading_error
        X[3][0] = (heading_error - last_heading_error) / dt

        last_lateral_error = lateral_error
        last_heading_error = heading_error

        K, _, _= lqr.dlqr(Ad, Bd, Q, R)
        feedback_angle = cal_feed_back_angle(K, X)
        feedforward_angle = cal_feed_forward_angle(dm.vx, plan_crt[target_pts_idx], K[0][3])
        wheel_angle = feedback_angle + feedforward_angle
        print("angle:", feedback_angle, feedforward_angle, wheel_angle)

        wheel_angle = max(-pi / 6, min(wheel_angle, pi / 6))

        dm.update_state(0, wheel_angle)

        
        plt.scatter([dm.x], [dm.y], s=1, label="car")
        plt.pause(0.2)
    plt.show()

if __name__ == "__main__":
    run()
