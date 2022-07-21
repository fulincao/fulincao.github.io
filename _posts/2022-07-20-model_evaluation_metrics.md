---
title: 机器学习模型评估指标
date: 2022-07-20
categories: [机器学习]
tags: [混淆矩阵, 机器学习, 分类, 回归]     # TAG names should always be lowercase
---

做了很多次模型评估，在此记录一下。
模型评估主要分为两类: **分类**，**回归**

## 分类
分类常用的指标有:
- Accuracy（准确率)
- Precision（精确率）
- Recall, Sensitivity（召回率，真正率，查全率，TPR）
- Specificity （FPR，假正率）
- F-Score （F值，Recall和Precision的调和值）
- ROC曲线（Receiver Operating Characteristic，横轴为FPR，纵轴为TPR）
- AUC （Area Under Curve，ROC曲线下的面积)

首先根据定义计算混淆矩阵

| 实际\预测|正|负 |
|----| ----|---- |
|正| TP	|  FN |
|负| FP	|  TN|

- **True Positive (真正，TP)**：将正类预测为正类数
- **True Negative(真负，TN)**：将负类预测为负类数
- **False Positive(假正，FP)**：将负类预测为正类数
- **False Negative(假负，FN)**：将正类预测为负类数

然后计算各指标  
- **Accuracy**
$$
\begin{aligned}
   Accuracy = \dfrac{TP + TN}{TP + FN + FP + TN}
\end{aligned}
$$

- **Precision**
$$
\begin{aligned}
   Precision = \dfrac{TP}{TP + FP}
\end{aligned}
$$

- **Recall(TPR)**
$$
\begin{aligned}
   Recall = \dfrac{TP}{TP + FN}
\end{aligned}
$$

- **FPR**
$$
\begin{aligned}
   FPR = \dfrac{FP}{FP + TN}
\end{aligned}
$$

- **F-score**
$$
\begin{aligned}
   F-score &= \dfrac{(1 + \beta^2)Precision * Recall}{\beta^2Precision + Recall} \\
   F1-score &= 2*\dfrac{Precision * Recall}{Precision + Recall} \qquad(\beta = 1)
\end{aligned}
$$

- **ROC曲线**
  - 按照分类阈值从0到1分别计算不同的TPR和FPR，以FPR为横轴，TPR为纵轴将这些点连起来如此
  - ROC曲线能够尽量降低不同测试集带来的干扰，更加客观地衡量模型本身的性能

- **AUC**
  - ROC曲线下的面积为AUC值
  - AUC值越大的分类器，正确率越高

## 回归
回归常用的指标有:
- **MSE（Mean Square Error,均方误差）**
$$
\begin{aligned}
  MSE = \dfrac{1}{n}\sum_{i=1}^{n}(\hat{y_i} - y_i)^2
\end{aligned}
$$

- **RMSE（Root Mean Square Error, 均方根误差）**
$$
\begin{aligned}
  RMSE = \sqrt{\dfrac{1}{n}\sum_{i=1}^{n}(\hat{y_i} - y_i)^2}
\end{aligned}
$$
- **MAE（Mean Absolute Error，平均绝对误差)**
$$
\begin{aligned}
  MAE = \dfrac{1}{n}\sum_{i=1}^{n}|\hat{y_i} - y_i|
\end{aligned}
$$
