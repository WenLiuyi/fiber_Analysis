#include <opencv.hpp>
#include <random>
#include<iostream>
#include<vector>
#include <fstream>
#include <cmath>
#include <map>
#include <algorithm>
#include <iomanip>
#include "visualize.h"

using namespace std;
using namespace cv;
namespace fs = std::filesystem;
constexpr double PI = 3.14159265358979323846;       //编译时计算的常量

//std::string resultFolder = "C:\\Users\\16584\\Desktop\\result_pictures";

// 定义高斯核函数
double gauss(double x) {
    return (1.0 / sqrt(2 * PI)) * exp(-0.5 * (x * x));
}
double get_kde(double x, vector<double>& data_array, double bandwidth) {
    int N = data_array.size();
    double res = 0.0;

    for (double val : data_array) {
        res += gauss((x - val) / bandwidth);
    }
    res /= (N * bandwidth);
    return res;
}
double saveKde(vector<vector<Point>>& clusters) {           //传回max_x：max_x拟合曲线极值点所对应的聚类所含的像素点数目
    vector<double> cluster_set; // 用于储存初分割聚类所含的像素点个数的列表
    for (const auto& cluster : clusters) {
        cluster_set.push_back(cluster.size());
    }

    // 设置高斯核函数带宽
    double sum = 0.0;
    double sum_squared = 0.0;
    int n = cluster_set.size();

    // 计算标准差
    for (int size : cluster_set) {
        sum += size;
        sum_squared += size * size;
    }

    double mean = sum / n;
    double variance = (sum_squared / n) - (mean * mean);
    double std_dev = sqrt(variance);

    double bandwidth = 1.05 * std_dev * pow(n, -1.0 / 5.0);

    // 设置 x_array 的范围
    double min_val = *min_element(cluster_set.begin(), cluster_set.end());
    double max_val = *max_element(cluster_set.begin(), cluster_set.end());

    vector<double> x_array;
    vector<double> y_array;

    // 生成 x_array 和对应的 y_array
    for (double x = min_val; x <= max_val; x += (max_val - min_val) / 180) {
        x_array.push_back(x);
        y_array.push_back(get_kde(x, cluster_set, bandwidth));
    }

    // 计算最大值及其对应的 x 值
    auto max_it = max_element(y_array.begin(), y_array.end());
    int max_idx = distance(y_array.begin(), max_it);
    double max_x = x_array[max_idx];
    double max_y = *max_it;

    //matplotlibcpp绘图
    return max_x;
}

void saveHistogram(vector<vector<Point>>& clusters, string filename) {
    size_t count = clusters.size(); // 初分割聚类滤去小聚类后，所含的聚类数目
    cout << "过滤后聚类数目：" << count << endl;

    //绘制聚类所含像素点数目分布直方图
    // 1. 计算每个聚类的大小
    std::vector<int> clusterSizes;
    for (const auto& cluster : clusters) {
        clusterSizes.push_back(cluster.size());
    }

    // 2. 计算直方图
    int maxSize = *std::max_element(clusterSizes.begin(), clusterSizes.end());
    int minSize = *std::min_element(clusterSizes.begin(), clusterSizes.end());
    //int maxValue = *std::max_element(clusterSizes.begin(), clusterSizes.end());     //所有聚类中的最多像素点个数
    int numBins = 30;
    int binSize = (maxSize + 30 - 1) / 30;     //直方图横坐标分为30等份，binWidth为每一等份的大小
    //std::vector<int> histogram(binSize, 0);

    // 创建直方图数组
    std::vector<int> histogram(numBins, 0);

    // 填充直方图
    for (int value : clusterSizes) {
        histogram[value / binSize]++;
    }

    // 3.创建绘图图像
    int histWidth = 600;
    int histHeight = 400;
    cv::Mat histImage(histHeight, histWidth, CV_8UC3, cv::Scalar(255, 255, 255));   //白底

    // 4.绘制直方图
    cout << "histWidth:" << histWidth << ", histogram size:" << histogram.size() << endl;
    int binWidth = cvRound((double)histWidth / 30);        //直方图中，每一等份在图像上的长度，这里是：600/30=20
    cout << "binWidth:" << binWidth << endl;
    for (size_t i = 0; i < histogram.size(); i++) {
        cv::rectangle(histImage,
                      cv::Point(i * binWidth, histHeight),
                      cv::Point((i + 1) * binWidth, histHeight - histogram[i]),
                      cv::Scalar(0, 0, 255), cv::FILLED);
    }

    // 5.添加标签
    cv::putText(histImage, "Cluster Size", cv::Point(histWidth / 2 - 50, histHeight - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    cv::putText(histImage, "Count", cv::Point(10, 20),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);

    // 保存图像
    cv::imwrite(filename, histImage);
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
            imwrite(resultFolder + "\\pre\\dyed\\" + name + "_cluster_" + to_string(NUM) + "_cnt_" + to_string(F) + ".png", img_single_rgb);
        }
        NUM++;
        imwrite(resultFolder +"\\pre\\segment\\" + name +"_segment" + ".png", mask);
    }

    // 4. 保存所有聚类的图像和输出信息
    imwrite(resultFolder + +"\\pre\\dyed_segment\\" + name + "_pre_segment" + ".png", img_rgb);
    cout << "获得纤维预分割聚类染色图" << endl;
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
    cout << "过滤前聚类数：" << clusters.size() << "，过滤后聚类数：" << filtered_clusters.size() << endl;

    vector<vector<Point>>clusters1 = filtered_clusters; // 用于初分割聚类染色
    size_t count = filtered_clusters.size();

    saveHistogram(filtered_clusters, resultFolder + "\\pre\\histogram\\" + name + "_histogram.png");        //聚类所含像素点数目分布直方图
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
    GaussianBlur(validImage1, gausBlur1, Size(11, 11), 1.5); //高斯卷积核计算图像均值
    GaussianBlur(validImage2, gausBlur2, Size(11, 11), 1.5);
    GaussianBlur(image1_2, gausBlur12, Size(11, 11), 1.5);

    Mat imageAvgProduct = gausBlur1.mul(gausBlur2); //均值乘积
    Mat u1Squre = gausBlur1.mul(gausBlur1); //各自均值的平方
    Mat u2Squre = gausBlur2.mul(gausBlur2);

    Mat imageConvariance, imageVariance1, imageVariance2;
    Mat squreAvg1, squreAvg2;
    GaussianBlur(image1_1, squreAvg1, Size(11, 11), 1.5); //图像平方的均值
    GaussianBlur(image2_2, squreAvg2, Size(11, 11), 1.5);

    imageConvariance = gausBlur12 - gausBlur1.mul(gausBlur2);// 计算协方差
    imageVariance1 = squreAvg1 - gausBlur1.mul(gausBlur1); //计算方差
    imageVariance2 = squreAvg2 - gausBlur2.mul(gausBlur2);

    auto member = ((2 * gausBlur1.mul(gausBlur2) + C1).mul(2 * imageConvariance + C2));
    auto denominator = ((u1Squre + u2Squre + C1).mul(imageVariance1 + imageVariance2 + C2));

    Mat ssim_map;
    divide(member, denominator, ssim_map);
    return mean(ssim_map)[0]; // 返回均值
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
