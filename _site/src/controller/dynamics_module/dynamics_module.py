'''
Description: file content
Author: caofulin@minieye.cc
FilePath: /fulincao.github.io/src/controller/dynamics_module.py
Date: 2022-01-20 20:40:21
'''

import math
import scipy.linalg as la
from numpy import fromiter

L = 4  # [m]
Lr = L / 2.0  # [m]
Lf = L - Lr
Cf = 1600.0 * 2.0  # N/rad
Cr = 1700.0 * 2.0  # N/rad
Iz = 1500 * 8  # kg * m^2 =  m_front * l_f^2 + m_rear * l_r^2
M = 1500.0 # kg

class DynamicsModel(object):

    def __init__(self, x=0.0, y=0.0, yaw=0.0, vx=0.01, vy=0.0, omega=0.0):
        self.x = x
        self.y = y
        self.yaw = yaw
        self.vx = vx
        self.vy = vy
        self.omega = omega


    def update_state(self, a, delta, dt=0.1):
        self.x = self.x + self.vx * math.cos(self.yaw) * dt - self.vy * math.sin(self.yaw) * dt
        self.y = self.y + self.vx * math.sin(self.yaw) * dt + self.vy * math.cos(self.yaw) * dt
        self.yaw = self.yaw + self.omega * dt
        Ffy = -Cf * math.atan2(((self.vy + Lf * self.omega) / self.vx - delta), 1.0)
        Fry = -Cr * math.atan2((self.vy - Lr * self.omega) / self.vx, 1.0)
        
        self.vx = self.vx + (a - Ffy * math.sin(delta) / M + self.vy * self.omega) * dt

        self.vy = self.vy + (Fry / M + Ffy * math.cos(delta) / M - self.vx * self.omega) * dt
        self.omega = self.omega + (Ffy * Lf * math.cos(delta) - Fry * Lr) / Iz * dt
        return self

    def get_state(self):
        return self.x, self.y, self.yaw, self.v



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

    # A[1][1] -= -(Cf + Cr) / (M * vx)
    # A[1][2] -= (Cf + Cr) / M
    # A[1][3] -= (-Cf * Lf + Cr * Lr) / (M * vx)


    A[2][3] = 1
    
    A[3][1] = -(Cf * Lf - Cr * Lr) / (Iz * vx)
    A[3][2] = (Cf * Lf - Cr * Lr) / Iz
    A[3][3] = -(Cf * Lf * Lf + Cr * Lr * Lr) / ( Iz * vx)

    # A[3][1] -= -(Cf * Lf - Cr * Lr) / (Iz * vx)
    # A[3][2] -= (Cf * Lf - Cr * Lr) / Iz
    # A[3][3] -= -(Cf * Lf * Lf + Cr * Lr * Lr) / ( Iz * vx)

    B[1][0] = Cf / M
    B[3][0] = Cf * Lf / Iz

    # B[1][0] -= Cf / M
    # B[3][0] -= Cf * Lf / Iz


    """
    离散变化 https://baike.baidu.com/item/%E5%8F%8C%E7%BA%BF%E6%80%A7%E5%8F%98%E6%8D%A2/19123826?fr=aladdin
    Ad = (1 + A*T/2) / (1 - A*T/2)
    """
    # I = np.eye(4, dtype=np.float64)
    # Ad = (I + A * dt / 2) @ la.inv( I - A* dt / 2 )
    # Bd = B * dt


    return A * dt, B * dt

class TempModel(object):
    def __init__(self, x=0.0, y=0.0, yaw=0.0, vx=0.01, vy=0.0, omega=0.0):
        self.x = x
        self.y = y
        self.yaw = yaw
        self.vx = vx
        self.vy = vy
        self.omega = omega
        self.state_vec = np.zeros((4, 1), dtype=np.float64)
        self.state_vec[0] = self.y
        self.state_vec[1] = self.vy
        self.state_vec[2] = self.yaw
        self.state_vec[3] = self.omega

    def update_state(self, a, delta, dt=0.1):
        
        A, B = set_AB(self.vx, dt)
        self.x = self.x + self.vx * math.cos(self.yaw) * dt - self.vy * math.sin(self.yaw) * dt
        self.state_vec = A @ self.state_vec + B * delta
        self.vx = self.vx + a*dt
        self.yaw = self.state_vec[2]
        self.y = self.state_vec[0]
        self.vy = self.state_vec[1]
        
        return self

    def get_state(self):
        return self.x, self.y, self.yaw, self.v


if __name__ == '__main__':
    print("start Kinematic Bicycle model simulation")
    import matplotlib.pyplot as plt
    import numpy as np

    T = 200
    a = [0] * T
    a[0] = 5
    delta = [math.radians(30)] * T
    #  print(a, delta)
    
    state = DynamicsModel()
    # state = TempModel(vx=1)
    # state.vx = 1
    x = []
    y = []
    yaw = []
    vx, vy = [], []
    time = []
    t = 0.0
    dt = 0.1  # [s]
    for (ai, di) in zip(a, delta):
        t = t + dt
        state.update_state(ai, di)
        x.append(state.x)
        y.append(state.y)
        yaw.append(state.yaw)
        vx.append(state.vx)
        vy.append(state.vy)
        time.append(t)

    flg, ax = plt.subplots(1)
    plt.plot(x, y)
    plt.xlabel("x[m]")
    plt.ylabel("y[m]")
    plt.axis("equal")
    plt.grid(True)

    flg, ax = plt.subplots(1)
    plt.scatter(time, np.array(vx) )
    plt.xlabel("Time")
    plt.ylabel("velocity[m/s]")
    plt.grid(True)

    #  flg, ax = plt.subplots(1)
    #  plt.plot([math.degrees(ibeta) for ibeta in beta])
    #  plt.grid(True)

    plt.show()