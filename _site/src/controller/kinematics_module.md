# 运动学模型

## 自行车模型
---
>  [自行车模型](https://blog.csdn.net/adamshan/article/details/78696874)
> 
自行车模型基于如下几个假设：

- 车辆在垂直方向的运动被忽略掉了，也就是说我们描述的车辆是一个二维平面上的运动物体（可以等价与我们是站在天空中的俯视视角）
- 假设车辆的结构就像自行车一样，也就是说车辆的前面两个轮胎拥有一直的角度和转速等，同样后面的两个轮胎也是如此，那么前后的轮胎就可以各用一个轮胎来描述
- 我们假设车辆运动也和自行车一样，这意味着是前面的轮胎控制这车辆的转角  

![bicycle_model](../assert/bicycle_model.jpeg)


### 自行车运动学模型
---  
作为一种自行车模型，运动学自行车模型也假定车辆形如一辆自行车，整个的控制量可以简化为$\alpha, \delta_f$.其中$\alpha$是车辆的加速度，踩油门踏板意味着正的加速度，踩刹车踏板意味着负的加速度。$\delta_f$是我们的方向盘转角，我们假定这个方向盘转角就是前轮胎当前的转角。这样，我们使用两个量描述了车辆的控制输入(control input)。

![bicycle_model](../assert/bicycle_model.png)

各变量说明如下：  
- $\delta_f$ 前轮转角
- $\psi$ 横摆角，车辆方向
- $v$速度方向
- $\beta$ 速度方向与车辆方向夹角
- $l_r, l_f$ 质点到前后轴的距离
- $x, y$模型的位置

其中模型的状态量为：$\psi, v, x, y$，更新公式如下：
$$ 
    x_{t+1} = x_t + v*\cos(\psi + \beta) * \Delta{t} \\
    y_{t+1} = y_t + v*\sin(\psi + \beta) * \Delta{t} \\
    v_{t+1} = v_t + \alpha * \Delta{t} \\
    \psi_{t+1} = \psi_t + \frac{v_t}{l_r} * \sin(\beta) * \Delta{t} \\
    \beta = \arctan(\frac{l_r}{l_r+l_f} * \tan(\delta_f))
$$
其中关于$\beta$和$\psi$的推导如下:  
$$
\begin{aligned}
    \tan(\delta_f) &= \dfrac{l_r+l_f}{R} \\
    \Rightarrow R &=  \dfrac{l_r + l_f}{\tan(\delta_f)} \\

    \tan(\beta) &= \dfrac{l_r}{R} \\
    \Rightarrow \beta &= \arctan(\dfrac{l_r}{R}) \\
                      &= \arctan(\dfrac{l_r}{l_r + l_f} * \tan(\delta_f)) \tag{1}

\end{aligned}
$$
$$
\begin{aligned}
    \sin(\beta) * v &= \omega * l_r \\
    \Rightarrow \omega &=  \dfrac{\sin(\beta)*v}{l_r} \\

    \psi_{t+1} &= \psi_t + \omega * \Delta{t} \\
            &= \psi_t + \dfrac{\sin(\beta)*v}{l_r} *  \Delta{t} \tag{2}\\
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
```
