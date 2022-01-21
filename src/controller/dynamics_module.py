'''
Description: file content
Author: caofulin@minieye.cc
FilePath: /fulincao.github.io/src/controller/dynamics_module.py
Date: 2022-01-20 20:40:21
'''

import math

L = 4  # [m]
Lr = L / 2.0  # [m]
Lf = L - Lr
Cf = 1600.0 * 2.0  # N/rad
Cr = 1700.0 * 2.0  # N/rad
Iz = 1500 * 8  # kg * m2 =  m_front * l_f^2 + m_rear * l_r^2
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

if __name__ == '__main__':
    print("start Kinematic Bicycle model simulation")
    import matplotlib.pyplot as plt
    import numpy as np

    T = 2000
    a = [0] * T
    # a[0] = 1
    delta = [math.radians(30)] * T
    #  print(a, delta)
    
    state = DynamicsModel()
    state.vx = 1
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
    plt.plot(time, np.array(vx) * 3.6)
    plt.xlabel("Time[km/h]")
    plt.ylabel("velocity[m]")
    plt.grid(True)

    #  flg, ax = plt.subplots(1)
    #  plt.plot([math.degrees(ibeta) for ibeta in beta])
    #  plt.grid(True)

    plt.show()