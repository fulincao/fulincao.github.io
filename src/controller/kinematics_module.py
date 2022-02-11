'''
Description: file content
Author: caofulin@minieye.cc
FilePath: /fulincao.github.io/src/controller/kinematics_module.py
Date: 2022-01-20 20:39:06
'''

import math

class KinematicModel(object):
    def __init__(self, x, y, yaw, v, f_len, r_len):
        self.x = x
        self.y = y
        self.yaw = yaw
        self.vx = v

        self.f_len = f_len
        self.r_len = r_len

    def get_state(self):
        return self.x, self.y, self.yaw, self.vx

    def update_state(self, a, delta, dt):
        beta = math.atan((self.r_len / (self.r_len + self.f_len)) * math.tan(delta))
        self.x = self.x + self.vx * math.cos(self.yaw + beta) * dt
        self.y = self.y + self.vx * math.sin(self.yaw + beta) * dt
        self.yaw = self.yaw + (self.vx / (self.f_len + self.r_len)) * math.cos(beta) * math.tan(delta) * dt
        self.vx = self.vx + a * dt
        return self.x, self.y, self.yaw, self.vx