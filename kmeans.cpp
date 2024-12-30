#include <vector>
#include <opencv.hpp>
#include <random>
#include <iostream>
#include <cmath>
#include <fstream>
#include<numeric>

#include<QString>

#include <tbb/blocked_range.h>
#include<tbb/parallel_for.h>
#include<atomic>
#include <tbb/concurrent_vector.h>

#include "kmeans.h"
#include "visualize.h"

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

constexpr double PI = 3.14159265358979323846;       //编译时计算的常量


double calculateDistance(const Point& a, const Point& b) {
    return norm(a - b);     //欧几里得距离
}

bool ClusterDivide(int clusterID, int split_number, vector<Point>& cluster,double resolution, double threshold, double ssim, double max_x, int circle_count,
                   int height, int width, string resultFolder, string name, vector<int> *repair_klist, vector<double>* diameter_list, vector<double>* percentage_list, bool revised) {
    bool need_revise = false;

    //1.随机选取中心点索引
    //从range(len(cluster))随机选取split_number个数，作为初始中心点的序号索引
    vector<int> array(cluster.size());
    iota(array.begin(), array.end(), 0); // 填充 array 为 [0, 1, 2, ..., n-1]
    random_device rd;
    mt19937 gen(rd());
    shuffle(array.begin(), array.end(), gen);

    vector<Point> initial_centers; // 存储初始聚类中心
    vector<Point> random_center;

    //2. 随机选择聚类中心:
    //（1）选择cluster中起始聚类中心点序号
    for (int G = 0; G < split_number; ++G) {
        random_center.push_back(cluster[array[G]]);
    }
    //（2）根据初始中心点的序号索引，选取cluster的split_number个像素点作为起始聚类中心
    for (const auto& center : random_center) {
        initial_centers.push_back(center);
    }

    /*
    *   initial_centers为初始聚类中心；
        new_cluster为重新分类的聚类集合；
        clusters_list为k近邻划分聚类列表，centers_list为k近邻划分聚类中心列表。
    */
    vector<vector<vector<Point>>> clusters_list; /*存放每次迭代后的聚类结果
                                                      例：[cluster_all[d], [[(3,2)], [(5,4)], [(6,7)]]]*/
    vector<vector<Point>> centers_list;     /* 存放每次迭代后的聚类中心坐标
                                        其中第一个元素是初始随机选取的聚类中心。例：[[(1,2), (3,4), (5,6)], [(3,2), (5,4), (6,7)]] */

    centers_list.push_back(initial_centers);
    /*注：对于clusters_list储存的第一个（初始）元素格式为[(1,2),(1,4),(1,5)]，其余元素格式类似是[[(1,2),(1,4)],[(1,5)]]都是相同的，
        第一个元素格式和其余元素格式不相同
        对于centers_list储存的所有元素格式相同始终都是类似[(1,2),(1,4),(1,5)]*/

    for (int iter_num = 0; iter_num < circle_count; iter_num++) {        //开始迭代
        vector<vector<Point>> current_clusters_list(split_number); // 存放 k 近邻再分割结果
        vector<Point> previous_centers = centers_list[iter_num];    //上一轮迭代产生的新聚类中心

        //3. 计算距离并分配点
        for (size_t s = 0; s < cluster.size(); ++s) {
            Point tuple = cluster[s];          // 取出该点
            vector<double> distance_list;       //距离列表：储存该点与聚类中心的距离

            // 计算与每个中心的距离
            for (const auto& center : previous_centers) {
                distance_list.push_back(calculateDistance(tuple, center));
            }

            double min_distance = *min_element(distance_list.begin(), distance_list.end());
            vector<int> distance_indexes;       //储存距离最近的聚类序号

            for (int M = 0; M < distance_list.size(); ++M) {
                if (distance_list[M] == min_distance) {
                    distance_indexes.push_back(M); // 找到最近的聚类索引
                }
            }

            // 根据最近距离分配点
            if (distance_indexes.size() > 1) {          //如果存在两个及以上的最近聚类(即到两个聚类中心的距离相等），进行随机数筛选，选择一个存入random_number列表中
                uniform_int_distribution<int> dist(0, distance_indexes.size() - 1);
                int random_index = distance_indexes[dist(gen)];
                current_clusters_list[random_index].push_back(tuple);
            }
            else {          // 只含一个索引数，选择该聚类
                current_clusters_list[distance_indexes[0]].push_back(tuple);
            }
        }
        clusters_list.push_back(current_clusters_list);

        Mat local_mask_gray = Mat::zeros(height, width, CV_8UC3);           // 初始化mask图像

        //4. 对每个k近邻划分的新聚类进行遍历,重新计算聚类中心
        vector<Point> current_centers;

        for (int P = 0; P < split_number; ++P) {
            if (current_clusters_list[P].size() > 1) {           // 新聚类所含元素数目＞1：重新计算聚类中心
                vector<int> x, y;
                for (const auto& point : current_clusters_list[P]) {
                    x.push_back(point.x);
                    y.push_back(point.y);
                }
                Point new_center(static_cast<int>(accumulate(x.begin(), x.end(), 0) / x.size()),
                                 static_cast<int>(accumulate(y.begin(), y.end(), 0) / y.size()));
                current_centers.push_back(new_center);
            }
            else {                                              // 新聚类所含元素数目=0：沿用旧聚类中心
                if (!current_clusters_list[P].empty()) {
                    current_centers.push_back(current_clusters_list[P][0]);
                }
                else {
                    current_centers.push_back(previous_centers[P]);
                }
            }
        }
        centers_list.push_back(current_centers);
        vector<vector<vector<Point>>> clusters_list_copied = deepcopyClustersList(clusters_list);

        // 5.检查聚类中心是否变化：聚类中心不随迭代次数iter_num改变，则循环迭代结束
        if (current_centers == previous_centers) {
            bool res = compareWithSSIM(centers_list, clusters_list_copied, clusterID, iter_num, split_number, ssim, resolution, height, width,
                                       resultFolder, name, repair_klist, diameter_list, percentage_list, revised);
            if (res == true) need_revise = true;
            break;
        }
    }
    return need_revise;
}

bool reSplit(vector<vector<Point>>& clusters, double threshold, int circle_count, double max_x, double ssim, double resolution,
             int height, int width, string resultFolder, string name,
             vector<int>* repair_klist, vector<double>* diameter_list, vector<double>* percentage_list, bool revised) {
    //参数：clusters 是初分割后的聚类列表. threshold是再分割阈值, iteration_num是最大迭代次数
    //bool need_revise=false;

    //vector<int> repair_klist; // 需要修正聚类序号指标列表
    //vector<double> diameter_list; // 聚类直径列表
    //vector<int> diameter_num_list; // 聚类直径列表直径对应聚类对应序号

    size_t count = clusters.size();        //聚类数目

    std::atomic<bool> need_revise(false); // 使用原子变量来确保线程安全的更新

    // 使用TBB的并行for循环来处理每个聚类
    tbb::parallel_for(tbb::blocked_range<int>(0, count), [&](const tbb::blocked_range<int>& r) {
        for (int d = r.begin(); d != r.end(); ++d) {
            int split_number = static_cast<int>(clusters[d].size() / (1.5 * max_x * threshold)) + 1;
            bool res = ClusterDivide(d, split_number, clusters[d], resolution, threshold, ssim, max_x, circle_count,
                height, width, resultFolder, name, repair_klist, diameter_list, percentage_list, revised);
            // 使用原子操作来更新need_revise
            if (res) {
                need_revise.store(true); // 设置为true是线程安全的
                // 注意：一旦设置为true，后续的操作就不会再改变它（假设我们不需要撤销这个设置）
            }
        }
        });

    // 对每个初分割聚类内部，进行再分割处理（其再分割聚类数spint_number由初分割聚类像素点数目、再分割单丝阈值共同决定）
    /*for (int d = 0; d < count; ++d) {
        int split_number = static_cast<int>(clusters[d].size() / (1.5 * max_x * threshold)) + 1;
        bool res = ClusterDivide(d, split_number, clusters[d], resolution, threshold, ssim, max_x, circle_count,
                                 height, width, resultFolder, name, repair_klist, diameter_list, percentage_list, revised);
        if (res) need_revise = true;
    }*/

    return need_revise;     // 是否需要修正
}

bool compareWithSSIM(vector<vector<Point>> centers_list, vector<vector<vector<Point>>> clusters_list_copied,
                     int clusterID, int iter_num, int split_number, double ssim, double resolution, int height, int width, string resultFolder, string name,
                     vector<int>* repair_klist, vector<double>* diameter_list,vector<double>* percentage_list, bool revised) {

    vector<int> area; // 存放split_number个新聚类所含点的数目
    for (int W = 0; W < split_number; W++) {
        area.push_back(clusters_list_copied[iter_num][W].size()); // 统计每个聚类的点数
    }

    vector<double> r; // 存放等效直径的列表
    for (int X = 0; X < split_number; X++) {
        double radius = sqrt(static_cast<double>(area[X]) / PI); // 计算等效半径
        r.push_back(radius);
    }

    // 图1：将初始某一聚类再分割的每一聚类进行白色染色，并分别保存图片
    Mat local_mask_gray = Mat::zeros(height, width, CV_8UC3);
    for (int Y = 0; Y < split_number; Y++) {
        Mat Tem_local_mask_gray = Mat::zeros(height, width, CV_8UC3); // 创建空白图像
        while (!clusters_list_copied[iter_num][Y].empty()) {
            auto point = clusters_list_copied[iter_num][Y].back(); // 获取聚类点
            clusters_list_copied[iter_num][Y].pop_back(); // 移除点

            // 将对应位置的像素设为白色
            for (int c = 0; c < 3; c++) {
                Tem_local_mask_gray.at<Vec3b>(point.x, point.y)[c] = 255;
                local_mask_gray.at<Vec3b>(point.x, point.y)[c] = 255;
            }
        }
        // 保存每个聚类的分割结果
        //string filename=resultFolder + "\\final\\segment\\" + name + "_cluster_" + to_string(clusterID) + "_subCluster_" + to_string(Y)+ ".png";
        //imwrite(filename, Tem_local_mask_gray);
    }
    //图2：将当前聚类的所有子聚类进行白色染色，保存为：[name]_cluster_[clusterID]_iteration_[iter_num].png
    string original_pic_filename = resultFolder + "\\final\\cir\\" + name + "_cluster_" + to_string(clusterID)  + ".png";
    imwrite(original_pic_filename, local_mask_gray);

    //图3：用圆拟合每个子聚类，对圆区域进行白染色，保存为：[name]_cluster_[clusterID]_iteration_[iter_num]_circled.png
    Mat original_pic = imread(original_pic_filename, IMREAD_UNCHANGED);
    Mat highlight_pic = imread(original_pic_filename, IMREAD_UNCHANGED);
    for (int z = 0; z < split_number; z++) {
        // 遍历每个子聚类：对上述白色染色图和黑色背景画圆
        circle(highlight_pic, Point(centers_list[iter_num][z].y, centers_list[iter_num][z].x), static_cast<int>(r[z]), Scalar(0, 0, 255), 3);
        circle(original_pic, Point(centers_list[iter_num][z].y, centers_list[iter_num][z].x), static_cast<int>(r[z]), Scalar(255, 255, 255), -1);
    }
    // 保存图像；original_pic是圆拟合后图片；highlight_pic是圆拟合后，对圆边界加粗后的图片
    string highlighted_pic_filename = resultFolder + "\\final\\cir\\" + name + "_cluster_" + to_string(clusterID) + "_circle_highlighted.png";
    string circled_pic_filename = resultFolder + "\\final\\cir\\" + name + +"_cluster_" + to_string(clusterID)  + "_circled.png";
    imwrite(highlighted_pic_filename, highlight_pic);
    imwrite(circled_pic_filename, original_pic);

    // 检查圆拟合前后的两图像ssim，是否超过阈值
    double ssim_result=ouputSSIMResult(original_pic_filename, circled_pic_filename, resultFolder, name, true);
    double percentage = ssim_result;

    if (revised == false) {       // 单丝再分割
        // 创建txt文件保存信息
        ofstream file(resultFolder + "\\" + name + "_processData.txt", ios::app);
        file << "\n初分割第" << clusterID << "聚类再分割结构相似性指数为" << percentage;

        for (int l = 0; l < split_number; l++) {
            file << "\n圆的中心为" << '(' << centers_list[iter_num][l].y << ',' << centers_list[iter_num][l].x << ')';
            file << "\n圆的半径为" << (r[l] * resolution) << "微米";
        }

        file.close(); // 关闭文件

        if (percentage <= ssim) {           //需要修正：将聚类序号保存至修正列表
            repair_klist->push_back(clusterID);
            return true;
        }
        else {
            for (int l = 0; l < split_number; l++) {        // 无需修正：保存等效直径
                diameter_list->push_back(r[l] * 2 * resolution);
            }
            return false;
        }
    }

    else {      // 单丝再分割的第一次修正
        if (percentage >= ssim) {
            percentage_list->push_back(percentage);

            ofstream file(resultFolder + "\\" + name + "_processData.txt", ios::app);
            if (file.is_open()) {
                file << "\n初分割第" << clusterID << "聚类修正再分割结构相似性指数为" << percentage;
                for (int l = 0; l < split_number; l++) {
                    file << "\n圆的中心为" << '(' << centers_list[iter_num][l].y << ',' << centers_list[iter_num][l].x << ')';
                    file << "\n圆的半径为" << (r[l] * resolution) << "微米";
                    diameter_list->push_back(r[l] * 2 * resolution);        // 保存等效直径
                }
                file.close();
            }
            else {
                cerr << "无法打开文件!" << endl;
            }
            return false;
        }
        else {      // 不满足阈值条件：需要进行第二次修正，并删去第一次修正结果的图
            percentage_list->push_back(percentage); // 添加结构相似性指数
            return true;
        }
    }
}

//深拷贝
vector<vector<vector<Point>>> deepcopyClustersList(vector<vector<vector<Point>>>& clusters_list) {
    vector<vector<vector<Point>>> new_clusters_list;

    // 遍历 clusters_list 的每一层，进行深拷贝
    for (const auto& cluster2D : clusters_list) {
        vector<vector<Point>> tempCluster2D;
        for (const auto& cluster1D : cluster2D) {
            std::vector<Point> tempCluster1D;
            // 进行深拷贝
            for (const auto& point : cluster1D) {
                tempCluster1D.push_back(point); // 利用 Point 的复制构造函数
            }
            tempCluster2D.push_back(tempCluster1D);
        }
        new_clusters_list.push_back(tempCluster2D);
    }

    return new_clusters_list;

}

vector<QString> parallel_second_repair(
    vector<int> repair_second_klist,
    vector<vector<Point>> filtered_clusters,
    double max_x,
    vector<vector<Point>> extractedClusters_2,
    int height,
    int width,
    string resultFolder,
    string name,
    vector<int> redundant_list,
    vector<double> diameter_list,
    vector<double> percentage_list,
    double threshold,
    int iteration_num,
    double resolution,
    double ssim,
    int circle_count)
{
    size_t len = repair_second_klist.size();
    vector<QString> logMessages;

    // 使用 tbb::parallel_for 来并行执行每个聚类的处理
    tbb::parallel_for(tbb::blocked_range<int>(0, len), [&](const tbb::blocked_range<int>& r) {
        tbb::concurrent_vector<std::string> localLogMessages;  // 局部日志存储

        for (int d = r.begin(); d != r.end(); ++d) {
            int local_split_number = static_cast<int>(filtered_clusters[repair_second_klist[d]].size() / (1.5 * max_x * threshold)) + 1;
            bool local_need_second_revise = true;

            while (local_need_second_revise) {
                bool res = true; // 假设初始结果为 true，实际应根据 ClusterDivide 的返回值设置
                for (int i = 0; i < iteration_num; i++) {
                    bool res = ClusterDivide(repair_second_klist[d], local_split_number, extractedClusters_2[d], resolution, threshold, ssim, max_x, circle_count,
                        height, width, resultFolder, name, &redundant_list, &diameter_list, &percentage_list, true);
                    if (!res) {
                        local_need_second_revise = false;
                        break;
                    }
                }
                if (local_need_second_revise) {
                    local_split_number += 1;
                }
            }

            // 收集线程局部日志
            std::ostringstream oss;
            oss << "聚类" << repair_second_klist[d] << "的第二次修正，修正后子聚类数目为：" << local_split_number;
            localLogMessages.push_back(oss.str());
        }

        // 将局部日志逐条添加到全局日志中
        for (const auto& log : localLogMessages) {
            logMessages.push_back(QString::fromStdString(log));
        }
        });

    return logMessages;
}


