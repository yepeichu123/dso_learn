# dso_learn
Each module code of DSO.


### DSO Module

&ensp; &ensp; 嗨，为了秉承我们[博客](https://blog.csdn.net/Y_Dumpling/article/details/112256719)开始前的诺言，笔者会尽可能地为各个模块提供可测试的代码，但是笔者能力有限，有所不足的地方还望各位读者能够不吝赐教，共同交流。


#### 1. 概述

&ensp; &ensp; 第一讲：[DSO全家桶（一）——DSO流程概述](https://blog.csdn.net/Y_Dumpling/article/details/112256719)主要是对DSO做了一个基本的介绍，同时也描述了笔者计划的博客大纲，这里并不需要写到任何代码。因此我们会从第二讲开始。


#### 2. PixelSelector

&ensp; &ensp; 第二讲：[DSO全家桶（二）——DSO前端：提取梯度点](https://blog.csdn.net/Y_Dumpling/article/details/112256721)主要是介绍DSO对于像素梯度点的提取流程。在DSO的跟踪阶段，为了节省时间，使用到了图像金字塔。因此，在梯度点的提取阶段，对于点的提取也是按照图像金字塔的标准提取的。这一讲，我们会提供配套的代码。

&ensp; &ensp; 具体的算法流程如下所示：
> a. 读取图像：首先会先检测输入变量是否合理，并且是否读取成功，读取成功则显示图像，否则就退出系统；

> b. 

