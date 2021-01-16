# PixelSelector——Designed by 叶培楚

### 简要说明

> 为了让读者朋友们更好地理解博客[DSO全家桶（二）——DSO前端：提取梯度点](https://blog.csdn.net/Y_Dumpling/article/details/112256721)的内容，笔者对DSO的提取梯度点模块进行重构，并从DSO代码中解耦出来，使之成为一个可以独立运行的模块。可以参考src/testPixelSelector.cpp这个demo，稍后我们会对算法流程做一个简单的介绍。为了让读者朋友们理解地更深刻，各个模块我们都配备了输出函数，大家按需使用！期待大家关注我博客，给我github点星哇！


### 依赖项

1. OpenCV 3.2.0

2. Eigen



### 编译和运行

非常简单的编译方法。。。
> mkdir build

> cd build

> cmake ..

> make -j2

> ./bin/testPixelSelector ./data/test.png



### 具体的算法流程如下所示：
> **读取图像**：首先会先检测输入变量是否合理，并且是否读取成功，读取成功则显示图像，否则就退出系统；
<div align=center>
    <img src=./data/test.png>
</div>

> **构建图像金字塔**：构建图像金字塔，包括计算给定网格尺寸32*32的直方图，图像金字塔，梯度图像和梯度平方和等金字塔；

<div align=center>
    <img src=./data/level_0.png>
</div>

<div align=center>
    <img src=./data/level_1.png>
</div>

<div align=center>
    <img src=./data/level_2.png>
</div>

<div align=center>
    <img src=./data/level_3.png>
</div>

<div align=center>
    <img src=./data/level_4.png>
</div>

<div align=center>
    <img src=./data/level_5.png>
</div>

> **提取图像梯度点**：参考博客：[DSO全家桶（二）——DSO前端：提取梯度点](https://blog.csdn.net/Y_Dumpling/article/details/112256721)中对算法的介绍。
> 首先我们会分多种potential进行筛选，从最里层开始（即pot*pot)，若找不到适合的像素，则依次往外查找(2pot * 2pot --> 4pot * 4pot)。
> 如果查找的像素过多或者过少，我们都会重新进行依次筛选。
<div align=center>
    <img src=data/imgWithSelectedPixels.png>
</div>


