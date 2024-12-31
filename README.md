# Log

## 1. UI设计

![8b0048360185445c820aca797367819c](file:///C:/Users/16584/Pictures/Typedown/8b004836-0185-445c-820a-ca797367819c.png?msec=1735619333137)

### 1.1 主操作区

![f2a2f934c1764b22a2153c421a684b03](file:///C:/Users/16584/Pictures/Typedown/f2a2f934-c176-4b22-a215-3c421a684b03.png?msec=1735619655813)

1. **Back，To**分别控制目录树的前进与回退；

2. 点击**Start**，代表开始运行；

3. **文件、参数设置**当前无功能，留待后续扩展。

### 1.2 文件路径区

* 上栏为图片所在文件夹路径；下栏为输出文件夹所在路径。
  
  * 注：两栏均需选择文件夹。

![490d4959000a4d6d80b9dfb68b726b8c](file:///C:/Users/16584/Pictures/Typedown/490d4959-000a-4d6d-80b9-dfb68b726b8c.png?msec=1735619817564)

操作示例：

    点击选择按钮，选中对应文件夹，点击”确定“；

![28aa858c9f8e41b18bb8c302a0fd6f56](file:///C:/Users/16584/Pictures/Typedown/28aa858c-9f8e-41b1-8bb8-c302a0fd6f56.png?msec=1735619934408)

### 1.3 参数区

![393c93118aba4971823759ec8d17ef3f](file:///C:/Users/16584/Pictures/Typedown/393c9311-8aba-4971-8237-59ec8d17ef3f.png?msec=1735620023876)

参数含义如下：

1. 图像分辨率：用于单丝等效直径的计算，有量纲量，单位为微米/像素；

2. 图像再分割阈值系数：用于再分割的单丝阈值设置，无量纲量；

3. 图像结构相似性指数阈值：用于筛选再分割过程需要修正分割的单丝，无量纲量（**0~1之间**）；

4. 消除随机性分割错误迭代次数：用于消除算法原理的随机性缺陷，无量纲量。
* 操作错误提示：
  
  * 若未输入参数，或参数格式不符合要求，点击**Start**时，会跳出错误提示。

![ba97ccc9e5cc4e7b957fc8ddfce26fc6](file:///C:/Users/16584/Pictures/Typedown/ba97ccc9-e5cc-4e7b-957f-c8ddfce26fc6.png?msec=1735620238196)

### 1.4 目录树

主要操作：

* 文件夹对应项显示黑色，其他格式对应项显示灰色；
  
  * ![c1af0ca51131480f801269bca134c425](file:///C:/Users/16584/Pictures/Typedown/c1af0ca5-1131-480f-8012-69bca134c425.png?msec=1735620904020)

* 可通过**Back，To**操控前进或后退；
  
  * 单击某项，该项被选中，边框变蓝，此时点击**To**，若该项为文件夹，跳转至相应文件夹下；
    
    * ![7a3ed8ee83914b8c966d9f5e558d54e6](file:///C:/Users/16584/Pictures/Typedown/7a3ed8ee-8391-4b8c-966d-9f5e558d54e6.png?msec=1735620820266)
  
  * 双击某项，若该项为文件夹，跳转至相应文件夹下；
  
  * 双击某项，若该项为**txt或png**格式，其文本/图片内容在展示区展现；
  
  * 光标停留在某项，若该文件名称超出边框，会突出显示其全名；
    
    * ![3769c83b743641129fcc1ce0071ac1d4](file:///C:/Users/16584/Pictures/Typedown/3769c83b-7436-4112-9fcc-1ce0071ac1d4.png?msec=1735620729645)

* 选择图片路径、输出路径时，目录树会跳转至相应文件夹下；

### 1.5 展示区

在目录树中双击**txt, png**格式的文本/图片，其内容展示在该区域。

注：可滑动查看

* 查看txt文本：
  
  * ![17eed7cce21c473aaa8f2bb10af887a8](file:///C:/Users/16584/Pictures/Typedown/17eed7cc-e21c-473a-aa8f-2bb10af887a8.png?msec=1735621034831)

* 查看png图片：
  
  * ![ba4f40b274344afebd31b6b8f8e293a5](file:///C:/Users/16584/Pictures/Typedown/ba4f40b2-7434-4afe-bd31-b6b8f8e293a5.png?msec=1735621143787)

### 1.6 命令行区

展示处理流程，可滑动查看

![6d23d92665b9419c8a44e3b86056c981](file:///C:/Users/16584/Pictures/Typedown/6d23d926-65b9-419c-8a44-e3b86056c981.png?msec=1735621218038)

### 1.7 工作区

* 待补充，内容预期为：所有图片的统计参量

## 2. 输出文件内容

> 注：输出图片/文件较为耗时，后续可精简，从而优化性能

输出路径的文件夹下，包括：

* **pre**：
  
  * cluster_0：抽取0值对应的像素点，单个初分割聚类图
  
  * cluster_255：抽取255值对应的像素点，单个初分割聚类图
  
  * contour_0：抽取0值对应的像素点，单个初分割轮廓图
  
  * contour_255：抽取255值对应的像素点，单个初分割轮廓图
  
  * dyed：单个初分割聚类染色图
    
    * 命名：`{name}_cluster_{id}_cnt_{num}`，name为原始图片名称，id为聚类序号，num为当前聚类包含的像素点个数；
  
  * dyed_segment：聚类染色图
    
    * 命名：`{name}_pre_segment`，name为原始图片名称
  
  * histogram：（预分割滤去小聚类后）聚类大小直方图
    
    * 命名：`{name}_histogram`，name为原始图片名称
  
  * processed：原始图像二值化处理后的图像
    
    * 命名：`{name}_binary`，name为原始图片名称
  
  * segment：原始图像分割后的图像
    
    * 命名：`{name}_segment`，name为原始图片名称

* **final**：
  
  * cir：
    
    1. 所有子聚类白色染色图：
       
       * 命名：`{name}_cluster_{id}`，name为原始图片名称，id为聚类序号
    
    2. 圆拟合每个子聚类，对圆区域进行白染色：
       
       * 命名：`{name}_cluster_{id}_circled`，name为原始图片名称，id为聚类序号
    
    3. 在2的基础上，标出圆区域的红色边框：
       
       * 命名：`{name}_cluster_{id}_circle_highlighted`，name为原始图片名称，id为聚类序号
  
  * cluster
  
  * diameter images：单丝等效直径分布直方图
    
    * 命名：`{name}_fiberDiameterImg`，name为原始图片名称
  
  * diameter texts：最终分割拟合单丝的等效直径信息，包括：每个聚类的单丝等效直径，单丝个数，单丝等效直径的均值、方差
    
    * 命名：`{name}_fiberDiameterData`，name为原始图片名称
  
  * result：最终分割结果图片
    
    * 命名：`{name}_final`，name为原始图片名称
  
  * roundness images：
    
    1. 单丝面积周长特征比分布直方图：
       
       * 命名：`{name} Area-Perimeter_Ratio`，name为原始图片名称
    
    2. 单丝短长半轴之比分布直方图：
       
       * 命名：`{name} Short-to-Long_Axis_Ratio`，name为原始图片名称
    
    3. 单丝相对偏心距分布直方图：
       
       * 命名：`{name} Relative_Eccentricity`，name为原始图片名称
    
    4. 单丝直径短边比分布直方图：
       
       * 命名：`{name} Diameter_Short-to-Side_Ratio`，name为原始图片名称
    
    5. 单丝直径长边比分布直方图：
       
       * 命名：`{name} Diameter_Long-to-Side_Ratio`，name为原始图片名称
  
  * roundness texts：最终分割拟合单丝的圆度参量信息
    
    * 命名：`{name}_fiberRoundnessData`，name为原始图片名称
  
  * segment
  
  * segment contour
  
  * **Cross Section Roundness Table**：圆度参量信息统计表
    
    * 每个Sheet对应一张原始图片，其名字为原始图片名称
  
  * **Equivalent Diameters Table**：等效直径信息统计表
    
    * 每个Sheet对应一张原始图片，其名字为原始图片名称

* text_processData.txt：初分割后的聚类信息
  
  * 命名：`{name}_processData`，name为原始图片名称


