---
title: 曲率和航向的计算
date: 2022-01-07
categories: [运动控制]
tags: [曲率, 航向]     # TAG names should always be lowercase
---

# 曲率和航向
> [曲率](https://baike.baidu.com/item/%E6%9B%B2%E7%8E%87/9985286?fr=aladdin)
> [航向](https://baike.baidu.com/item/%E8%88%AA%E5%90%91/3923000?fr=aladdin)

曲线的曲率（curvature）就是针对曲线上某个点的切线方向角对弧长的转动率，通过微分来定义，表明曲线偏离直线的程度。数学上表明曲线在某一点的弯曲程度的数值。曲率越大，表示曲线的弯曲程度越大。曲率的倒数就是曲率半径。

设曲线直角坐标方程$y = f(x)$且具有二阶导数，那么曲率公式为：
$$
\begin{aligned}
    K = \dfrac{|y^{''}|}{(1+y^{'2})^{\dfrac{3}{2}} }
\end{aligned}
$$
航向为：
$$
\begin{aligned}
    Yaw = \arctan(y')
\end{aligned}
$$

如果曲线是由参数方程
$$
\begin{aligned}
    x &= \psi(t) \\
    y &= \omega(t)
\end{aligned}
$$
那么曲率为:
$$
\begin{aligned}
  K = \dfrac{|\psi'(t)\omega''(t) - \omega'(t)\psi''(t)|}{[\psi^{'2}(t) + \omega^{'2}(t)]^{\dfrac{3}{2}}}  
\end{aligned}
$$
那么航向为:
$$
\begin{aligned}
    Yaw = \arctan(\dfrac{\omega'(t)}{\psi'(t)})
\end{aligned}
$$
附录求导公式：
$$
\begin{aligned}
    \dfrac{dy}{dx} &= \dfrac{\dfrac{dy}{dt}}{\dfrac{dx}{dt}} = \dfrac{\omega'(t)}{\psi'(t)} \\
    \dfrac{d^2y}{dx^2} &= \dfrac{d}{dt}(\dfrac{dy}{dx})(\dfrac{1}{\dfrac{dx}{dt}}) = (\dfrac{dy}{dx})'(\dfrac{1}{\dfrac{dx}{dt}}) \\
    &=  (\dfrac{\omega'(t)}{\psi'(t)})'(\dfrac{1}{\psi'(t)}) \\
    &= \dfrac{\omega^{'2}(t)\psi^{'}(t) - \psi^{'2}(t)\omega^{'}(t)}{\psi^{'3}(t)}
\end{aligned}
$$
