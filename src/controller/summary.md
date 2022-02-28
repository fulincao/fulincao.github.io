---
title: 自行车模型
date: 2021-12-20
categories: [模型, 运动学模型]
tags: [模型, 运动学]     # TAG names should always be lowercase
---
# 运动学模型

## 自行车模型
---
>  [自行车模型](https://blog.csdn.net/adamshan/article/details/78696874)  
>  [车辆运动学模型](https://blog.csdn.net/u013914471/article/details/82968608)
> 
自行车模型基于如下几个假设：

- 车辆在垂直方向的运动被忽略掉了，也就是说我们描述的车辆是一个二维平面上的运动物体（可以等价与我们是站在天空中的俯视视角）
- 假设车辆的结构就像自行车一样，也就是说车辆的前面两个轮胎拥有一直的角度和转速等，同样后面的两个轮胎也是如此，那么前后的轮胎就可以各用一个轮胎来描述
- 我们假设车辆运动也和自行车一样，这意味着是前面的轮胎控制这车辆的转角  

![bicycle_model](../../assets/img/bicycle_model.jpeg)


### 自行车运动学模型
---  
作为一种自行车模型，运动学自行车模型也假定车辆形如一辆自行车，整个的控制量可以简化为$\alpha, \delta_f, \delta_r$.其中$\alpha$是车辆的加速度，踩油门踏板意味着正的加速度，踩刹车踏板意味着负的加速度。$\delta_f$是前轮转角，$\delta_r$是后轮转角。这样我们使用三个量描述了车辆的控制输入(control input)，其中状态量我们用$\psi, V, x, y$来表示。

![bicycle_model](../../assets/img/bicycle_model_2.png)

各变量说明如下：  
- $\delta_f$ 前轮转角
- $\delta_r$ 后轮转角
- $\psi$ 航向角
- $V$速度方向
- $\beta$ 速度方向与车辆方向夹角(滑移角)
- $l_r, l_f$ 质点到前后轴的距离
- $\omega$ 角速度
- $R$ 转弯半径
- $x, y$模型的位置

由三角函数可得：
$$
\begin{aligned}
    \dfrac{\sin(\delta_f - \beta)}{l_f} &=  \dfrac{\sin( \dfrac{\pi}{2} - \delta_f)}{R} \\
    \dfrac{\sin(\beta - \delta_r)}{l_r} &= \dfrac{\sin(\dfrac{\pi}{2} + \delta_r)}{R} \\
\end{aligned}
$$
展开：
$$
\begin{aligned}
    \dfrac{\sin\delta_f\cos\beta - \cos\delta_f\sin\beta}{l_f} &=  \dfrac{\cos\delta_f}{R} \qquad(1)\\
    \dfrac{\sin\beta\cos\delta_r - \cos\beta\sin\delta_r}{l_r} &= \dfrac{\cos\delta_r}{R} \qquad(2)\\
\end{aligned}
$$
可以求得$R, \beta$：  

0. $(1)$式同除$\delta_f$同乘$l_f$加上$(2)$式同除$\delta_r$同乘$l_r$得:
$$
\begin{aligned}
    \dfrac{l_f + l_r}{R} &= \cos\beta(\tan\delta_f - \tan\delta_r) \\
    R &= \dfrac{l_f + l_r}{\cos\beta(\tan\delta_f - \tan\delta_r)} \qquad(3) \\
\end{aligned}
$$
1. $(1)$式除以$(2)$式：
$$
\begin{aligned}
    \dfrac{\sin\delta_f\cos\beta - \cos\delta_f\sin\beta}{\sin\beta\cos\delta_r - \cos\beta\sin\delta_r} &=  \dfrac{\cos\delta_f * l_f}{\cos\delta_r * l_r} \\
\end{aligned}
$$
2. 等式同时除以$\cos\delta_f$，乘以$\cos\delta_r$得:  
$$
\begin{aligned}
    \dfrac{\tan\delta_f\cos\beta - \sin\beta}{\sin\beta - \cos\beta\tan\delta_r} &=  \dfrac{l_f}{l_r} \\
\end{aligned}
$$
3. 等式左边同时除以$\cos\beta$得:  
$$
\begin{aligned}
    \dfrac{\tan\delta_f - \tan\beta}{\tan\beta - \tan\delta_r} &=  \dfrac{l_f}{l_r} \\
\end{aligned}
$$
4. 合并整理得:  
$$
\begin{aligned}
    \tan\beta &=  \dfrac{l_r * \tan\delta_f + l_f * \tan\delta_r}{l_f + l_r} \\
    \beta &= \arctan(\dfrac{l_r * \tan\delta_f + l_f * \tan\delta_r}{l_f + l_r}) \qquad(4) \\
\end{aligned}
$$

低速环境下，车辆行驶路径的转弯半径变化缓慢，此时我们可以假设车辆的方向变化率等于车辆的角速度。则车辆的角速度$\omega$为
$$
\begin{aligned}
    \omega &= \dfrac{V}{R} \\
\end{aligned}
$$
代入$(3)$得:
$$
\begin{aligned}
    \omega &= \dfrac{V * \cos\beta * (\tan\delta_f - \tan\delta_r)}{l_f + l_r} \qquad(5)\\
\end{aligned}
$$
所以模型的状态量$x, y, \psi, V$的更新如下:  
$$
\begin{aligned}
    x_{t+1} &= x_t + V_t*\cos(\psi_t + \beta) * \Delta{t} \\
    y_{t+1} &= y_t + V_t*\sin(\psi_t + \beta) * \Delta{t} \\
    V_{t+1} &= V_t + \alpha * \Delta{t} \\
    \psi_{t+1} &= \psi_t + \omega * \Delta{t} \\
\end{aligned}
$$

当仅前轮驱动时，$\delta_r$可认为是0则$\psi, V, x, y$，更新公式如下：
$$
\begin{aligned}
    x_{t+1} &= x_t + V*\cos(\psi_t + \beta) * \Delta{t} \\
    y_{t+1} &= y_t + V*\sin(\psi_t + \beta) * \Delta{t} \\
    V_{t+1} &= V_t + \alpha * \Delta{t} \\
    \psi_{t+1} &= \psi_t + \frac{V_t}{l_f + l_r} * \cos(\beta) * \tan(\delta_f) * \Delta{t} \\
    \beta &= \arctan(\frac{l_r}{l_r+l_f} * \tan(\delta_f))
\end{aligned}
$$

### 代码实现
---

```python3
class KinematicModel(object):
    def __init__(self, x, y, psi, v, f_len, r_len):
        self.x = x
        self.y = y
        self.psi = psi
        self.v = v
`   
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
```


## 车辆动力学模型
---
>  [动力学模型](https://blog.csdn.net/u013914471/article/details/83018664)  


动力学主要研究作用于物体的力与物体运动的关系，车辆动力学模型一般用于分析车辆的平顺性和车辆操纵的稳定性。对于车来说，研究车辆动力学，主要是研究车辆轮胎及其相关部件的受力情况。比如纵向速度控制，通过控制轮胎转速实现；横向航向控制，通过控制轮胎转角实现。

正常情况下，车辆上的作用力沿着三个不同的轴分布：
- 纵轴上的力包括驱动力和制动力，以及滚动阻力和拖拽阻力作滚摆运动；
- 横轴上的力包括转向力、离心力和侧风力，汽车绕横轴作俯仰运动；
- 立轴上的力包括车辆上下振荡施加的力，汽车绕立轴作偏摆或转向运动


![dynamics_car_module](../../assets/img/dynamics_car.png)

而在单车模型假设的前提下，再作如下假设即可简单搭建车辆的动力学模型：  
- 只考虑纯侧偏轮胎特性，忽略轮胎力的纵横向耦合关系；  
- 用单车模型来描述车辆的运动，不考虑载荷的左右转移；
- 忽略横纵向空气动力学。

其中需要考虑的受力点有：
- 横向运动：曲线行驶时的离心力，侧偏力等
- 纵向运动：受总驱动阻力、加速、减速等的影响。总驱动阻力由滚动阻力、拖拽阻力和坡度阻力等构成。


### 横向车辆动力学
---  
![lateral_dynamics_module](../../assets/img/lateral_dynamics_module.png)

各变量说明如下：  
- $m, m_f, m_r$整车质量，前挂质量，后挂质量
- $\delta_f, \delta_r$ 前后轮转角
- $\alpha_f, \alpha_r$ 前后轮滑移角
- $C_{\alpha f}, C_{\alpha r}$ 前后轮侧偏刚度
- $\psi，\dot{\psi}$ 航向角，角速度
- $V_x, V_y$ 纵横向速度
- $F_{yf},F_{yr}$ 前后轮y方向受力
- $F_{cf},F_{cr}$ 前后轮侧向力
- $l_f, l_r$ 前后轴长度
- $I_z=m_f * l_{f}^2 + m_r * l_{r}^2$转动惯量

在y轴上：
$$
\begin{aligned}
    ma_y = F_{yf} + F_{y_r} \qquad(1)
\end{aligned}
$$
在z轴上：
$$
\begin{aligned}
    I_z\ddot{\psi} = l_f * F_{yf} - l_r * F_{yr} \qquad(2)
\end{aligned}
$$
y轴方向加速由y轴方向位移相关的加速度$\ddot{y}$和向心加速度$V_x\dot{\psi}$构成：
$$
\begin{aligned}
    a_y = \ddot{y} + V_x\dot{\psi} \qquad(3)
\end{aligned}
$$
即:
$$
\begin{aligned}
    m(\ddot{y} + V_x\dot{\psi}) =  F_{yf} + F_{y_r} \qquad(4)
\end{aligned}
$$

轮胎收到横向压力，会产生很小的滑移角
![lateral_dynamics_module](../../assets/img/slip_angle.png)

则前后轮滑移角(默认前轮驱动，后轮不动即后轮偏移角为0):
$$
\begin{aligned}
    \alpha_f &= \delta_f - \theta_{vf} \\
    \alpha_r &= - \theta_{vr}
\end{aligned}
$$
即前后轮所受横向力为：
$$
\begin{aligned}
    F_{yf} &= 2C_{\alpha f}(\delta_f - \theta_{vf}) \qquad(5)\\
    F_{yr} &= 2C_{\alpha r}(-\theta_{vr}) \qquad(6) \\
\end{aligned}
$$
其中$\theta_{vf},\theta_{vr}可以通过y方向速度和切线速度计算$：
$$
\begin{aligned}
    \tan(\theta_{vf}) = \dfrac{V_y + l_f\dot{\psi}}{V_x} \\
    \tan(\theta_{vr}) = \dfrac{V_y - l_r\dot{\psi}}{V_x}
\end{aligned}
$$
在小角度下，正切函数和正比例函数相似，因此可以近似为：
$$
\begin{aligned}
    \theta_{vf} = \dfrac{V_y + l_f\dot{\psi}}{V_x} \qquad(7) \\
    \theta_{vr} = \dfrac{V_y - l_r\dot{\psi}}{V_x} \qquad(8)
\end{aligned}
$$
将(5),(6),(7),(8)带入(1)(2)中得到:
![推导](../../assets/img/tuidao.jpg)

整理得到动力学模型:
![横向动力学](../../assets/img/lateral_dynamics_state.png)


### 代码实现
---

```python3
'''
动力学模型模拟
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
```
