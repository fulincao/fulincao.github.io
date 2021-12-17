from math import atan, cos, sqrt
from os import path
from matplotlib.colors import PowerNorm
from pyproj import Proj
from math import pi, sin, cos
import math
import numpy as np
import matplotlib.pyplot as plt

DEG_TO_RAD = math.pi / 180


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
        self.psi = self.psi + (self.v / self.f_len) * math.sin(beta) * dt
        self.v = self.v + a * dt
        return self.x, self.y, self.psi, self.v

def read_global_path(file_log):
    """[summary]
    Args:
        file_log ([str]): log path

    Returns:
        [list]: [ [lng, lat, alt, yaw], ... ]  in deg
    """
    res = []
    vx = 0
    rf = open(file_log, "r")
    for line in rf:
        line = line.strip()
        fields = line.split(",")
        if "$GINS" in fields[0] and len(fields) > 10:
            lat, lng, alt = float(fields[3]), float(fields[4]), float(fields[5])
            yaw = (float(fields[11]) + 360) % 360
            res.append([lng, lat, alt, vx, yaw])

        if "$GNRMC" in fields[0] and len(fields) > 10:
            vx = float(fields[7]) * 0.774



    return res

def test(log_path):
    proj_text = "+proj=utm +zone=50 +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"
    trsfm = Proj(proj_text)
    path_point = read_global_path(log_path)

    x, y = [], []
    for point in path_point:
        utm_x, utm_y = trsfm(point[0], point[1])
        x.append(utm_x)
        y.append(utm_y)
    x = np.array(x)
    y = np.array(y)

    km = KinematicModel(0, 0, 0, 0, 0, 0)

    yaw_list = []
    lat_erros = []

    model_x = []
    model_y = []

    # 113.745098853 22.737577497
    # km.x, km.y = trsfm(113.745098853, 22.737577497)
    km.x, km.y = x[0], y[0]
    km.v =  10 / 3.6 # path_point[0][-2]  m/s
    km.psi = pi/2 - path_point[0][-1] * DEG_TO_RAD
    
    km.r_len = 1.5
    km.f_len = 1.5

    target_pts_idx = 0
    iter = 0

    kp, ki, kd = 0.01, 10, 0
    g_rate = 0.1

    last_error = -1
    last_last_error = -1

    while iter < 100000:
        if target_pts_idx >= len(x):
            break
        # km.v = path_point[target_pts_idx][-2]
        iter += 1
        ld = max(km.v * g_rate, 2)
        dx = km.x - x[target_pts_idx]
        dy = km.y - y[target_pts_idx]
        euler_dist = math.sqrt(dx**2 + dy**2)

        if euler_dist < ld:
            target_pts_idx += 1
            continue

        print(target_pts_idx, ld, euler_dist, dx, dy, km.x, km.y)
    
        cos_target_heading = cos(path_point[target_pts_idx][-1] * DEG_TO_RAD)
        sin_target_heading = sin(path_point[target_pts_idx][-1] * DEG_TO_RAD)

        # lateral_error = cos_target_heading * dy - sin_target_heading * dx
        # print("lateral_error", lateral_error)
        lateral_error = cos_target_heading * dx - sin_target_heading * dy 
        longtidual_error = sin_target_heading * dx + cos_target_heading * dy
        
        print("lateral_error", lateral_error)
        # print("longtidual_error", longtidual_error)
        # print("euler_dist:", math.sqrt(dx**2 + dy**2))

        alpha = atan(2 * 2.9 * lateral_error / euler_dist**2) + kp * lateral_error + ki * (lateral_error - last_error) + kd * (lateral_error - 2*(last_error) + last_last_error)
        alpha = min(alpha, 30 * DEG_TO_RAD)
        alpha = max(alpha, -30 * DEG_TO_RAD)
        last_last_error = last_error
        last_error = lateral_error
        # print("ld:", ld)
        print("alpha:", alpha)
        km.update_state(0, alpha, 0.01)
        model_x.append(km.x)
        model_y.append(km.y)


    # plt.plot(x, lat_erros)
    # plt.plot(x, yaw_list)
    plt.scatter(model_x, model_y, label="model", s=1)
    plt.scatter(x, y, label="true", s=1)
    plt.legend()
    plt.show()


def main():
    # zone mean utm zone , beijing is 50
    proj_text = "+proj=utm +zone=50 +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"
    P = Proj(proj_text)

    point_init = (22.534624368,113.949931864, 5.720, -173.04 * DEG_TO_RAD)
    point_current = (22.526802456, 113.949383126, 6.151, -173.04 * DEG_TO_RAD)
    point_target = (22.529278272, 113.949620569, 5.518, pi/3)
    


    utm_init = P(point_init[1], point_init[0],)
    utm_current = P(point_current[1], point_current[0])
    utm_target = P(point_target[1], point_target[0])

    print("utm_init:", utm_init)
    print("utm_current:", utm_current)
    print("utm_target:", utm_target)

    x_diff_map = utm_current[0] - utm_init[0]
    y_diff_map = utm_current[1] - utm_init[1]
    theta_diff = point_current[-1] - point_init[-1]

    cos_map_veh = cos(point_init[-1])
    sin_map_veh = sin(point_init[-1]);

    x_diff_veh = cos_map_veh * x_diff_map + sin_map_veh * y_diff_map;
    y_diff_veh = -sin_map_veh * x_diff_map + cos_map_veh * y_diff_map;

    print("current_veh_x:", x_diff_veh)
    print("current_veh_y:", y_diff_veh)

    cos_theta_diff = cos(-theta_diff)
    sin_theta_diff = sin(-theta_diff)

    tx = -(cos_theta_diff * x_diff_veh - sin_theta_diff * y_diff_veh)
    ty = -(sin_theta_diff * x_diff_veh + cos_theta_diff * y_diff_veh)

    print(tx, ty)


    x = utm_target[0]
    y = utm_target[1]
    theta = point_target[-1]

    x_new = cos_theta_diff * x - sin_theta_diff * y + tx
    y_new = sin_theta_diff * x + cos_theta_diff * y + ty
    theta_new = theta - theta_diff

    print("target_new_x:", x_new)
    print("target_new_y:", y_new)


    dx = utm_current[0] - utm_target[0]
    dy = utm_current[1] - utm_target[1]
    
    print("dx:", dx, "dy:", dy)

    cos_target_heading = cos(point_target[-1])
    sin_target_heading = sin(point_target[-1])

    lateral_error = cos_target_heading * dy - sin_target_heading * dx
    print("lateral_error", lateral_error)
    lateral_error = cos_target_heading * dx - sin_target_heading * dy
    print("lateral_error", lateral_error)

if __name__ == "__main__":
    
    # x = []
    # y = []
    # km = KinematicModel(0, 0, pi/3, 5, 0.5 ,0.5)
    # for i in range(200):
    #     km.update_state(0, 0, 0.1)
    #     x.append(km.x)
    #     y.append(km.y)
    #     plt.plot(x, y)
    #     plt.pause(0.1)
    test("cutecom.log")
    # main()