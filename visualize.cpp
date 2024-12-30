#undef ACCESS_MASK

#include <opencv.hpp>
#include <matplot/matplot.h>

#include <random>
#include<iostream>
#include<vector>
#include <fstream>
#include <cmath>
#include <map>
#include <algorithm>
#include <iomanip>
#include "visualize.h"

#include <QString>
#include "Runner.h"
#include "FolderManager.h"

//#include "matplotlibcpp.h"
//#include <Python.h>


#undef emit

using namespace std;
using namespace cv;
namespace fs = std::filesystem;
using namespace matplot;
//constexpr double PI = 3.14159265358979323846;       //编译时计算的常量

QString message;

//std::string resultFolder = "C:\\Users\\16584\\Desktop\\result_pictures";

// 函数：将路径中的反斜杠转为正斜杠
string convertPath(const string& path) {
    string convertedPath = path;

    // 替换所有的反斜杠为正斜杠
    for (char& c : convertedPath) {
        if (c == '\\') {
            c = '/';
        }
    }

    return convertedPath;
}

// 高斯核函数计算
double gauss(double x) {
    return (1.0 / std::sqrt(2 * M_PI)) * std::exp(-0.5 * x * x);
}
// 核密度估计函数
double getKde(double x, const vector<int>& data_array, double bandwidth) {
    double res = 0.0;
    size_t N = data_array.size();
    for (size_t i = 0; i < N; ++i) {
        res += gauss((x - data_array[i]) / bandwidth);
    }
    res /= (N * bandwidth);
    return res;
}
double saveKde(vector<vector<Point>>& clusters, const string& filename) {           //传回max_x：max_x拟合曲线极值点所对应的聚类所含的像素点数目
    // 1. 获取每个聚类的大小
    vector<int> cluster_sizes;      // 用于储存初分割聚类所含的像素点个数的列表
    for (const auto& cluster : clusters) {
        cluster_sizes.push_back(cluster.size());
    }

    // 2. 计算高斯核带宽
    double bandwidth = 1.05 * std::sqrt(std::accumulate(cluster_sizes.begin(), cluster_sizes.end(), 0.0, [](double sum, int size) {
        return sum + size * size;
        }) / cluster_sizes.size() - std::pow(std::accumulate(cluster_sizes.begin(), cluster_sizes.end(), 0.0) / cluster_sizes.size(), 2))
        * std::pow(cluster_sizes.size(), -1.0 / 5);

    // 3. 计算核密度估计
    vector<double> x_array;
    vector<double> y_array;
    int min_size = *std::min_element(cluster_sizes.begin(), cluster_sizes.end());
    int max_size = *std::max_element(cluster_sizes.begin(), cluster_sizes.end());

    for (double x = min_size; x <= max_size; x += (max_size - min_size) / 180.0) {
        x_array.push_back(x);
        y_array.push_back(getKde(x, cluster_sizes, bandwidth));
    }

    /*
    // 4. 绘制直方图
    auto h = bar(linspace(min_size, max_size, cluster_sizes.size()), cluster_sizes);
    title("聚类直方图与核函数密度估计");
    xlabel("聚类大小");
    ylabel("概率");

    // 5. 绘制核密度估计曲线，确保传入正确的类型
    // 如果需要设置样式，可以这样：
    plot(x_array, y_array, "r-"); // 红色线条

    // 6. 找到核函数密度曲线的极值点
    auto max_idx = std::max_element(y_array.begin(), y_array.end()) - y_array.begin();
    vector<double> max_x = { x_array[max_idx] };
    vector<double> max_y = { y_array[max_idx] };
    vector<double> red_color = { 1.0, 0.0, 0.0 };  // 红色
    scatter(max_x, max_y, 50, red_color);  // 散点图

    // 7. 添加图例
    vector<string> legend_labels = { "高斯核估计概率密度曲线", "概率密度极值点" };
    matplot::legend(legend_labels);

    // 8. 保存图像
    save(convertPath(filename));*/

    auto max_idx = std::max_element(y_array.begin(), y_array.end()) - y_array.begin();
    vector<double> max_x = { x_array[max_idx] };
    vector<double> max_y = { y_array[max_idx] };

    return x_array[max_idx]; // 返回最大值的 x 坐标
}


// 保存聚类直方图的函数
void saveClusterHistogram(vector<vector<cv::Point>>& clusters, string filename) {
    size_t count = clusters.size(); // 初分割聚类滤去小聚类后，所含的聚类数目
    std::cout << "过滤后聚类数目：" << count << endl;

    // 1. 计算每个聚类的大小
    std::vector<int> clusterSizes;
    for (const auto& cluster : clusters) {
        clusterSizes.push_back(cluster.size());
    }

    // 2. 设置 bin 大小，横轴以 200 为一个刻度
    int binSize = 200;
    int maxSize = *std::max_element(clusterSizes.begin(), clusterSizes.end());
    int numBins = (maxSize / binSize) + 1; // 计算需要多少个 bin

    // 3. 计算每个 bin 中的聚类数目
    std::vector<int> histogram(numBins, 0);
    for (int size : clusterSizes) {
        int binIndex = size / binSize; // 根据大小确定每个聚类属于哪个 bin
        histogram[binIndex]++;
    }

    // 4. 使用 matplot++ 绘制直方图
    auto h = linspace(0, (numBins - 1), numBins);  // 横轴为 0 到 numBins-1
    auto x = bar(h, histogram);                    // 绘制直方图，Y 轴为 histogram 中的聚类数目
    x->bar_width(0.8);                             // 设置柱形图的宽度

    // 设置横轴的位置
    vector<double> xticks(numBins);
    for (int i = 0; i < numBins; ++i) {
        xticks[i] = i * binSize;  // 每个 bin 的开始位置
    }
    matplot::xticks(xticks); // 设置横轴的刻度位置

    // 设置横轴标签为 200 的倍数
    vector<string> xlabels(numBins);
    for (int i = 0; i < numBins; ++i) {
        xlabels[i] = to_string(i * binSize); // 设置横轴标签为 200 * n
    }
    matplot::xticklabels(xlabels); // 设置横轴标签

    // 添加标题和标签
    matplot::title("聚类直方图（预分割滤去小聚类后）");
    matplot::xlabel("聚类大小");
    matplot::ylabel("聚类数目");

    // 保存图像为文件
    matplot::save(convertPath(filename));
}


/*参数列表：
*   clusters1: 一个二维向量，表示聚类结果。每个聚类包含一组Point对象。
    count: 聚类的数量。
    height和width: 图像的高度和宽度。
    resultFolder: 保存结果图像的文件夹路径。
    name: 图像文件的基本名称。
*/
void visualizeClusters(const vector<vector<Point>>& clusters1, size_t count, int height, int width, string &resultFolder ,const string& name) {
    Mat img_rgb = Mat::zeros(height, width, CV_8UC3);       //黑色图像，类型为3通道（RGB）
    Mat mask = Mat::zeros(height, width, CV_8UC1);          //黑色单通道掩模图像
    int NUM = 0;

    while (NUM < count) {
        if (clusters1[NUM].size() > 1) {
            // 1. 为当前聚类生成随机颜色
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 255);

            Vec3b E(dis(gen), dis(gen), dis(gen));

            // 2. 创建单个聚类的图像
            Mat img_single_rgb = Mat::zeros(height, width, CV_8UC3); // 黑色图像：用于单个聚类的可视化
            size_t F = clusters1[NUM].size(); // 当前聚类中点的数量

            for (const auto& point : clusters1[NUM]) {
                int j = point.x;
                int k = point.y;

                mask.at<uchar>(j, k) = 255; // 在掩模mask中将对应位置设置为255（白色），表示该位置属于当前聚类
                img_single_rgb.at<Vec3b>(j, k) = E; // 在img_single_rgb和img_rgb中，将该位置的颜色设置为随机生成的颜色E
                img_rgb.at<Vec3b>(j, k) = E;
            }

            // 3. 保存单个聚类图像和掩模
            cv::imwrite(resultFolder + "\\pre\\dyed\\" + name + "_cluster_" + to_string(NUM) + "_cnt_" + to_string(F) + ".png", img_single_rgb);
        }
        NUM++;
        cv::imwrite(resultFolder +"\\pre\\segment\\" + name +"_segment" + ".png", mask);
    }

    // 4. 保存所有聚类的图像和输出信息
    cv::imwrite(resultFolder + "\\pre\\dyed_segment\\" + name + "_pre_segment" + ".png", img_rgb);


    std::cout << "获得纤维预分割聚类染色图" << endl;
}

vector<vector<Point>> visualize(vector<vector<Point>>& clusters, string resultFolder,string name, double resolution, int height, int width) {
    double D = 3.5 / resolution;
    double area_threshold = 0.1 * (D * D);

    vector<vector<Point>>filtered_clusters;      //滤去部分小聚类
    for (auto& cluster : clusters) {
        if (cluster.size() > static_cast<int>(area_threshold)) {
            filtered_clusters.push_back(cluster);
        }
    }
    std::cout << "过滤前聚类数：" << clusters.size() << "，过滤后聚类数：" << filtered_clusters.size() << endl;

    vector<vector<Point>>clusters1 = filtered_clusters; // 用于初分割聚类染色
    size_t count = filtered_clusters.size();

    saveClusterHistogram(filtered_clusters, resultFolder + "\\pre\\histogram\\" + name + "_histogram.png");        //聚类所含像素点数目分布直方图
    //聚类染色图（待补充）
    visualizeClusters(clusters1, count, height, width, resultFolder, name);
    return filtered_clusters;
}

// 计算 SSIM 函数
double calculateSSIM(const cv::Mat& image1, const cv::Mat& image2, Mat& diff) {
    static const double C1 = 6.5025, C2 = 58.5225;

    Mat validImage1, validImage2;
    image1.convertTo(validImage1, CV_32F); //数据类型转换为 float,防止后续计算出现错误
    image2.convertTo(validImage2, CV_32F);

    Mat image1_1 = validImage1.mul(validImage1); //图像乘积
    Mat image2_2 = validImage2.mul(validImage2);
    Mat image1_2 = validImage1.mul(validImage2);

    Mat gausBlur1, gausBlur2, gausBlur12;
    cv::GaussianBlur(validImage1, gausBlur1, Size(11, 11), 1.5); //高斯卷积核计算图像均值
    cv::GaussianBlur(validImage2, gausBlur2, Size(11, 11), 1.5);
    cv::GaussianBlur(image1_2, gausBlur12, Size(11, 11), 1.5);

    Mat imageAvgProduct = gausBlur1.mul(gausBlur2); //均值乘积
    Mat u1Squre = gausBlur1.mul(gausBlur1); //各自均值的平方
    Mat u2Squre = gausBlur2.mul(gausBlur2);

    Mat imageConvariance, imageVariance1, imageVariance2;
    Mat squreAvg1, squreAvg2;
    cv::GaussianBlur(image1_1, squreAvg1, Size(11, 11), 1.5); //图像平方的均值
    cv::GaussianBlur(image2_2, squreAvg2, Size(11, 11), 1.5);

    imageConvariance = gausBlur12 - gausBlur1.mul(gausBlur2);// 计算协方差
    imageVariance1 = squreAvg1 - gausBlur1.mul(gausBlur1); //计算方差
    imageVariance2 = squreAvg2 - gausBlur2.mul(gausBlur2);

    auto member = ((2 * gausBlur1.mul(gausBlur2) + C1).mul(2 * imageConvariance + C2));
    auto denominator = ((u1Squre + u2Squre + C1).mul(imageVariance1 + imageVariance2 + C2));

    Mat ssim_map;
    cv::divide(member, denominator, ssim_map);
    return cv::mean(ssim_map)[0]; // 返回均值
}

double ouputSSIMResult(string beforePath, string afterPath, string resultFolder, string name, bool compare) {
    /*参数：repair_klist存储需要修复的聚类索引，diameter_list存储直径*/
    //1. 读取预分割前后的图片
    cv::Mat before = cv::imread(beforePath);
    cv::Mat after = cv::imread(afterPath);

    //2. 检查图片是否成功读取
    if (before.empty() || after.empty()) {
        std::cerr << "Error: Unable to open image files." << std::endl;
        return 0;
    }
    //3. 将图像转换为灰度图
    cv::Mat before_gray, after_gray;
    cv::cvtColor(before, before_gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(after, after_gray, cv::COLOR_BGR2GRAY);

    // 4. 计算 SSIM 指数
    Mat diff;                           //差异图
    double ssim= calculateSSIM(before_gray, after_gray, diff);
    //std::cout << ssim * 100 << "%" << std::endl;

    if (!compare) {
        // 5. 将 diff 转换为 8-bit
        //cv::Mat diff1;
        //result.diff.convertTo(diff1, CV_8U, 255.0);

        // 6. 储存预分割聚类参数信息
        std::ofstream file(resultFolder + "\\" + name + "_processData.txt");
        int count = 10; // 假设的聚类数量，根据实际情况修改
        file << "预分割筛选小聚类后的聚类个数共有:" << count << "个\n";
        file << "预分割筛选小聚类前后的结构相似性指数：" << ssim << "\n";
        file.close();

        std::cout << "储存预分割聚类信息" << std::endl;
    }
    return ssim;
}

//深拷贝
vector<vector<Point>> deepcopyClusters(const vector<vector<Point>>& clusters) {
    // 创建一个与原始 clusters 同样大小的新的二维 vector
    std::vector<std::vector<Point>> newClusters;
    newClusters.resize(clusters.size()); // 调整外层 vector 的大小

    // 深拷贝每一层的内容
    for (size_t i = 0; i < clusters.size(); ++i) {
        newClusters[i].resize(clusters[i].size()); // 调整内层 vector 的大小
        for (size_t j = 0; j < clusters[i].size(); ++j) {
            newClusters[i][j] = clusters[i][j]; // 逐个复制 Point 对象
        }
    }
    return newClusters; // 返回深拷贝后的二维 vector
}
