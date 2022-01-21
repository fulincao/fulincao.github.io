'''
Description: file content
Author: caofulin@minieye.cc
FilePath: /fulincao.github.io/src/controller/kinematics_module.py
Date: 2022-01-20 20:39:06
'''

import math

class KinematicModel(object):
    def __init__(self, x, y, psi, v, f_len, r_len):
        self.x = x
        self.y = y
        self.psi = psi
        self.v = v

        self.f_len = f_len
        self.r_len = r_len

    def get_state(self):
        return self.x, self.y, self.psi, self.v

    def update_state(self, a, delta, dt):
        beta = math.atan((self.r_len / (self.r_len + self.f_len)) * math.tan(delta))
        self.x = self.x + self.v * math.cos(self.psi + beta) * dt
        self.y = self.y + self.v * math.sin(self.psi + beta) * dt
        self.psi = self.psi + (self.v / (self.f_len + self.r_len)) * math.cos(beta) * math.tan(delta) * dt
        self.v = self.v + a * dt
        return self.x, self.y, self.psi, self.v