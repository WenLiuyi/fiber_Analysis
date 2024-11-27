# Log

## 1. UI设计

## 2. 环境配置

### 2.1 动态链接库引入

* **OpenCV**：[【C++】【OpenCV】Visual Studio 2022 配置OpenCV - VanGoghpeng - 博客园 (cnblogs.com)](https://www.cnblogs.com/vangoghpeng/p/18030685)

* **xlnt**：[windows 平台下 xlnt 结合visual studio 2017 用c++操作excel-CSDN博客](https://blog.csdn.net/s110730/article/details/82916136)

* **matplotlib-cpp**：[C++matplotlibcpp配置、使用及发布(第三方无需安装python即可运行)-CSDN博客](https://blog.csdn.net/weixin_44272195/article/details/126837083#:~:text=matplotl)[Visual Studio配置C++绘图库matplotlibcpp的方法 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/585302210#:~:text=%E6%9C%AC%E6%96%87%E4%BB%8B%E7%BB%8D%E5%9C%A8%20Vi)

> （1）OpenCV遇到过的问题：
> 
> 路径正确且不含中文，读取图片失败：`Debug`版的`exe`调用`Release`版的`lib`引起的。
> 
> （2）Python 3.11弃用`matplotlibcpp.h`中的 PySys_SetPath 和 Py_SetProgramName等：直接注释掉
> 
> （3）引入`matplotlib-cpp`：`numpy=1.23.0`

### 2.2 Qt

* **Qt Visual Studio Tools**：
  
  * 镜像：[关于Visual Studio下载Qt插件很慢的解决方案之一_在vs2019下载qt插件很慢-CSDN博客](https://blog.csdn.net/qq_44207723/article/details/107692766)
  
  * 在Visual Studio中配置：[如何联合Qt，VS，C++，来开发一个电脑版软件（简单有趣，详细） - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/685171588)

* 编译器版本：`6.7.0_msvc2019_64`

### 2.3 Anaconda环境配置

> `Anaconda Navigator`打不开：[Windows里Anaconda-Navigator无法打开的解决方案（详细）_为什么anaconda navit打不开-CSDN博客](https://blog.csdn.net/weixin_42529892/article/details/81503261)
