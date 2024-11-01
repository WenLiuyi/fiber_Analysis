#include "Runner.h"
#include "FolderManager.h"

#include <iostream>
#include <opencv.hpp>
#include <xlnt/xlnt.hpp>
#include <filesystem>
#include <vector>
#include <locale>
#include<algorithm>
#include<string>
#include <fstream>

#include <Windows.h>
#include <wchar.h>
#include <windows.h>

#include "visualize.h"
#include "createDirectoriesExcels.h"
#include "kmeans.h"

#include<QString>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

//函数声明
vector<vector<Point>> findClusters(Mat& matrix, int pixel);
void dfs(int y, int z, Mat & matrix, vector<vector<bool>>& visited, vector<Point>& cluster, int pixel);
vector<vector<Point>> deepcopyClusters(vector<vector<Point>>& clusters);
vector<double> processClusters(vector<vector<Point>>& clusters, const string& name, int height, int width, int pixel);
bool pixel_inverse(vector<double> list_255, vector<double> list_0, cv::Mat grayImg, string name, int height, int width, Mat& matrix);
bool ends_with(const string& str, const string& suffix);
void deletePictures(string resultFolder, string name, int clusterID);
vector<vector<Point>> extractClusters(vector<vector<Point>>& clusters, vector<int>* repair_klist);

double calculate_length_of_contour(const Mat& img);

std::string baseFolder = "";
std::string resultFolder = "";

QString Runner::getMessage() const {
    return m_message;
}

void Runner::setMessage(const QString& message) {
    qDebug() << "------setMessage";
    //m_message=message;
    emit updateLog(m_message);
}

void Runner::updateLog(const QString& message) {
    m_message = message;
    emit messageChanged();
}


QString Runner::performTask() {
    // 模拟一些任务，替代 run.cpp 中的逻辑
    //std::setlocale(LC_ALL, "");
    /*plt::plot({1,3,2,4});
 plt::show();
 std::cout << "Hello World!\n";*/

    /*1. 超参数输入：
     分辨率：等效直径计算
     再分割阈值系数：再分割参数设置
     结构相似性阈值：再分割错误修正
     消除随机性分割错误迭代次数：再分割错误修正
 */
    //double resolution, threshold, ssim;
    //int iteration_num;
    //std::cin >> resolution >> threshold >> ssim >> iteration_num;

    // 输出属性值
    qDebug() << "Resolution:" << resolution;
    qDebug() << "Threshold:" << threshold;
    qDebug() << "SSIM:" << ssim;
    qDebug() << "Iteration Number:" << iteration_num;

    QString baseFolder_qstring = m_folderManager->baseFolder();
    baseFolder = baseFolder_qstring.toStdString().substr(8);
    QString resultFolder_qstring = m_folderManager->resultFolder();
    resultFolder = resultFolder_qstring.toStdString().substr(8);

    qDebug() << "baseFolder:" << baseFolder;
    qDebug() << "resultFolder:" << resultFolder;

    createDirectories(resultFolder);

    //对文件夹里的每张图片进行遍历：
    try {
        for (const auto& entry : fs::directory_iterator(baseFolder)) {
            if (!fs::is_regular_file(entry.status())) continue;

            std::string extension = entry.path().extension().string();
            if (extension != ".png" && extension != ".jpg" && extension != ".jpeg") continue;

            std::string filename = entry.path().filename().string();
            //std::cout << filename << std::endl;
            //std::cout << "开始图片"<<filename<<"的分割处理:" << std::endl;
            QString message_1 = QString("开始图片 %1 的分割处理:").arg(QString::fromStdString(filename));  // Note1
            updateLog(message_1);

            std::string name = filename.substr(0, filename.find_last_of('.'));
            string diameter_excelPath = resultFolder + "\\final\\Equivalent Diameters Table.xlsx";
            string roundness_excelPath = resultFolder + "\\final\\Crosss Section Roundness Table.xlsx";
            // 创建保存直径计算数据的excel表
            createExcelFile(diameter_excelPath, { "Cluster Type", "Equivalent Diameter(micrometer)" });

            // 创建保存圆度计算数据的excel表："对应聚类类型", "面积周长特征比", "短长半轴比", "相对偏心距", "直径短边短边比", "直径长边比"
            //createExcelFile(resultFolder + "\\final\\Crosss Section Roundness Table.xlsx", { "Cluster Type", "S/L", "a/b", "d/D", "2R/A", "2R/B" });
            createExcelFile(roundness_excelPath, { "Cluster Type", "S/L", "a/b", "d/D", "2R/A", "2R/B" });


            // 2.图像预处理
            string path = baseFolder + "\\" + filename;

            // 2.1 灰度化处理
            cv::Mat grayImg = cv::imread(baseFolder + "\\" + filename, cv::IMREAD_GRAYSCALE);
            if (grayImg.empty()) {
                throw cv::Exception(0, "hello!", "main", "image.jpg", __LINE__);
            }
            int height = grayImg.rows;      //height, width分别为图像高度和宽度尺寸
            int width = grayImg.cols;
            std::cout << height << width << endl;

            // 2.2 图像滤波处理
            cv::GaussianBlur(grayImg, grayImg, cv::Size(3, 3), 0);  //高斯滤波

            // 2.3 图像二值化处理
            cv::Mat binaryImg;

            cv::threshold(grayImg, binaryImg, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU); //OTSU二值化
            cv::imwrite(resultFolder + "\\pre\\processed\\" + name + "_binary.png", binaryImg);       //保存二值化图像

            QString message_2 = QString("1. 图像预处理完成，输出二值化图像 %1 ").arg(QString::fromStdString(name+ "_binary.png"));  // Note2
            updateLog(message_2);

            //3. 单丝初分割及结果可视化

            // 3.1 图像像素矩阵获取
            Mat matrix = Mat::zeros(height, width, CV_8U);

            // 获取图片的二维像素信息，并将像素信息存入二维数组中
            for (int A = 0; A < height; ++A) {
                for (int B = 0; B < width; ++B) {
                    matrix.at<uchar>(A, B) = binaryImg.at<uchar>(A, B);
                }
            }

            // 3.2 单丝初分割及二值对比度修正
            auto clusters_255 = findClusters(matrix, 255);
            auto clusters_255_copy= deepcopyClusters(clusters_255);
            auto clusters_0 = findClusters(matrix, 0);
            auto clusters_0_copy = deepcopyClusters(clusters_0);

            std::cout << "完成纤维遍历";
            //list_255:像素值为255的初分割聚类的轮廓周长;list_0:像素值为0的初分割聚类的轮廓周长
            vector<double> list_255= processClusters(clusters_255, name, height, width,255);
            vector<double> list_0= processClusters(clusters_0, name, height, width,0);



            /*我们总是希望纤维对应像素二值化的结果为255，树脂、孔隙对应像素二值化结果为0.
         * 但由于拍摄条件影响，有时会出现树脂、孔隙对应像素二值化结果为255，纤维对应像素二值化的结果为0
         * 下面的判据就是为了消除这种影响，根据轮廓周长进行判断，基于树脂、孔隙对应轮廓周长总是大于纤维最大轮廓周长，若max(list_255) > max(list_0)
         * 则(list_0)对应是纤维的信息，需要对图像进行像素反转
         */
            Mat matrix_1;
            bool judge=pixel_inverse(list_255, list_0, grayImg, name,height,width, matrix_1);
            if (judge) {
                findClusters(matrix_1, 255);
            }

            // 3.3 初分割结果可视化：聚类所含像素点数目分布直方图， 聚类染色图
            // （滤去所含像素点少的聚类）
            vector<vector<Point>> filtered_clusters=visualize(clusters_255, resultFolder, name, resolution, height, width);
            
            QString message_3 = QString("2. 初分割聚类完成，初始聚类数目为%1，过滤后聚类数目为%2，输出聚类图至文件夹 %3，输出轮廓图至文件夹 %4 ")
                .arg(QString::number(clusters_255.size())).arg(QString::number(filtered_clusters.size())
                .arg(QString::fromStdString("/pre/cluster")).arg(QString::fromStdString("/pre/contour")));  // Note2
            updateLog(message_3);

            // 3.4 预分割的图像处理前后效果比较，获取SSIM指数
            ouputSSIMResult(resultFolder+"\\pre\\processed\\"+name+"_binary.png", resultFolder + "\\pre\\segment\\" + name + "_segment.png", resultFolder, name, false);

            QString message_4 = QString("3. 初分割聚类结果可视化，输出聚类染色图至 %1").arg(QString::fromStdString("/pre/dyed_segment"));  // Note3
            updateLog(message_4);

        //4. 再分割单丝分割阈值确定
            double max_x=saveKde(filtered_clusters);             //max_x拟合曲线极值点所对应的聚类所含的像素点数目
            cout << "获取高斯核图极值点，作为再分割单丝的分割阈值" << endl;

            QString message_5 = QString("4. 获取高斯核图极值点，作为再分割单丝的分割阈值");  // Note3
            updateLog(message_5);

            //5. 单丝再分割
            vector<int> repair_first_klist, repair_second_klist, redundant_list; // 存储需要修复的聚类索引（第一次/第二次修正）
            vector<double> diameter_list; // 存储直径
            vector<double> percentage_list; // 储存第一次修正过程中，处理的结构相似性指数
            int circle_count = 100;
            reSplit(filtered_clusters, threshold, circle_count, max_x, ssim, resolution, height, width, resultFolder, name, &repair_first_klist, &diameter_list, &percentage_list, false);

            QString message_6 = QString("5. 初次单丝再分割完整，待修正的聚类数为%1：").arg(QString::number(repair_first_klist.size()));  // Note3
            updateLog(message_6);

            //6. 单丝再分割过程修正
            vector<vector<Point>>clusters_k_repair = deepcopyClusters(filtered_clusters);
            vector<vector<Point>>clusters_kk_repair = deepcopyClusters(filtered_clusters);

            // 6.1 第一次再分割修正
            // 针对第一类错误：聚类数目split number正确，但因为k近邻方法所含随机性导致初始聚类中心选的不合适

            QString message_7 = QString("6. 开始第一次再分割修正");  // Note3
            updateLog(message_7);

            int len = repair_first_klist.size();
            cout << "第一次修正的初分割聚类数为：" << len << endl;
            vector<vector<Point>> extractedClusters_1 = extractClusters(filtered_clusters, &repair_first_klist);
            for (int d = 0; d < len; d++) {
                // 遍历每一个待修正的初分割聚类:
                int split_number= static_cast<int>(static_cast<int>(clusters_k_repair[repair_first_klist[d]].size() / (1.5 * max_x * threshold)) + 1);
                bool need_second_revise = true;

                // 1. 将该类的圆拟合图和圆填充图全部删去
                //deletePictures(resultFolder, name, repair_first_klist[d]);

                // 2. 判断结构相似性指数列表中的最大值，是否大于ssim阈值；
                // 若满足，修正过程结束，保存结果；若不满足，需要第二次修正。
                cout << "开始进入聚类" << to_string(repair_first_klist[d]) << "的第一次修正：" << endl;

                QString message_8 = QString("聚类%1 的第一次修正").arg(QString::number(repair_first_klist[d]));  // Note3
                updateLog(message_8);

                for (int i = 0; i < iteration_num; i++) {
                    cout << "iteration_" << to_string(i) << endl;
                    int split_number = static_cast<int>(filtered_clusters[repair_first_klist[d]].size() / (1.5 * max_x * threshold)) + 1;
                    bool res = ClusterDivide(repair_first_klist[d], split_number, extractedClusters_1[d], resolution, threshold, ssim, max_x, circle_count,
                                             height, width, resultFolder, name, &redundant_list, &diameter_list, &percentage_list, true);
                    if (res == false) { need_second_revise = false; break; }
                    //else deletePictures(resultFolder, name, repair_first_klist[d]);
                }
                if (need_second_revise) repair_second_klist.push_back(repair_first_klist[d]);
            }

            // 6.2 第二次再分割修正
            // 针对第二类错误，即聚类数目split_number的错误

            QString message_9 = QString("7. 开始第二次修正，待第二次修正的初分割聚类数为%1：").arg(QString::number(repair_second_klist.size()));  // Note3
            updateLog(message_9);

            cout << "第二次修正的初分割聚类数为：" << repair_second_klist.size() << endl;
            len = repair_second_klist.size();
            vector<vector<Point>> extractedClusters_2 = extractClusters(filtered_clusters, &repair_second_klist);
            for (int d = 0; d < len; d++) {
                int split_number = static_cast<int>(filtered_clusters[repair_second_klist[d]].size() / (1.5 * max_x * threshold)) + 1;
                bool need_second_revise = true;

                //deletePictures(resultFolder, name, repair_second_klist[d]);

                cout << "开始进入聚类" << to_string(repair_second_klist[d]) << "的第二次修正：" << endl;

                
                while (1) {
                    for (int i = 0; i < iteration_num; i++) {
                        bool res = ClusterDivide(repair_second_klist[d], split_number, extractedClusters_2[d], resolution, threshold, ssim, max_x, circle_count,
                                                 height, width, resultFolder, name, &redundant_list, &diameter_list, &percentage_list, true);
                        if (res == false) { need_second_revise = false; break; }
                        //else deletePictures(resultFolder, name, repair_second_klist[d]);
                    }
                    if (need_second_revise == false) break;
                    else split_number += 1;
                    cout << "split_number: " << to_string(split_number) << endl;
                }
                cout << "聚类" << to_string(repair_second_klist[d]) << "的子聚类数目："<< to_string(split_number) << endl;

                QString message_10 = QString("聚类%1 的第二次修正，修正后子聚类数目为：%2").arg(QString::number(repair_second_klist[d])).arg(QString::number(split_number));  // Note3
                updateLog(message_10);
            }

            //7. 单丝最终分割的结果可视化
            // 对单个染色和无染色的聚类图像进行合并，并保存合并图像
            int count = filtered_clusters.size();
            Mat img1= imread(resultFolder + "\\final\\cir\\" + name + "_cluster_" + to_string(0) + "_circled.png");
            Mat img2 = imread(resultFolder + "\\final\\cir\\" + name + "_cluster_" + to_string(1) + "_circled.png");
            if (img1.empty() || img2.empty()) {
                std::cerr << "无法读取图像文件" << std::endl;
                //return -1;
            }
            // 合并图像
            Mat res_pic;
            add(img1, img2, res_pic);

            int clusterID = 2;
            while (clusterID < count) {
                Mat img3 = imread(resultFolder + "\\final\\cir\\" + name + "_cluster_" + to_string(clusterID) + "_circled.png");
                if (img3.empty()) {
                    std::cerr << "无法读取图像文件: " << clusterID << std::endl;
                    clusterID++;
                    continue; // 如果图像读取失败，继续下一个
                }
                // 合并当前图像与结果图像
                cv::add(res_pic, img3, res_pic);
                clusterID++;
            }
            imwrite(resultFolder + "\\final\\result\\" + name + "_final.png", res_pic);

            //8. 单丝统计参量计算
            // 8.1 单丝等效直径
            //单丝等效直径直方图（待补充）
            string diameter_filePath = resultFolder + "\\" + name + "_fiberDiameterData.txt";
            save_diameter_info(diameter_filePath, diameter_excelPath, diameter_list, repair_first_klist);
        }
    }catch (const fs::filesystem_error& e) {
        cerr << "Filesystem error: " << e.what() << endl;
    }
    return 0;

    QString result = "Task completed successfully!";
    qDebug() << result; // 打印结果
    return result;
}

// 根据 repair_klist 提取 clusters 的子集
vector<vector<Point>> extractClusters(vector<vector<Point>>& clusters, vector<int>* repair_klist) {
    vector<vector<Point>> result;

    // 遍历 repair_klist 中的每个索引
    for (int index : *repair_klist) {
        // 检查索引是否在 clusters 范围内
        if (index >= 0 && index < clusters.size()) {
            result.push_back(clusters[index]); // 添加对应的子集
        }
    }
    return result;
}

// 需要修正，将当前的圆拟合图和圆填充图全部删去
void deletePictures(string resultFolder, string name,int clusterID) {
    for (const auto& entry : fs::directory_iterator(resultFolder + "\\final\\cir\\")) {
        std::string filename = entry.path().filename().string();
        if (filename.find(name + "_cluster_" + to_string(clusterID)) == 0) { // 寻找文件名以 [name]_cluster_[repair_klist[d]] 开头
            fs::remove(entry.path());
        }
    }
}

// 判断str是否以suffix结尾
bool ends_with(const string& str, const string& suffix) {
    if (str.length() >= suffix.length()) {
        return (0 == str.compare(str.length() - suffix.length(), suffix.length(), suffix));
    }
    return false;
}

bool pixel_inverse(vector<double> list_255,vector<double> list_0, cv::Mat grayImg, string name, int height, int width, Mat & matrix) {
    //cout << "compare:" << *max_element(list_255.begin(), list_255.end()) << ", " << *max_element(list_0.begin(), list_0.end()) << endl;
    if (*max_element(list_255.begin(), list_255.end()) <= *max_element(list_0.begin(), list_0.end())) return false;
    cv::Mat inverted;
    cv::bitwise_not(grayImg, inverted);  // 像素反转
    cv::imwrite(resultFolder + "\\pre\\pixel inverse\\" + name + ".png", inverted);  // 保存反转图片

    cv::GaussianBlur(inverted, inverted, cv::Size(3, 3), 0);  // 高斯滤波
    cv::threshold(inverted, inverted, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);  // OTSU二值化
    cv::imwrite(resultFolder + "\\pre\\processed\\" + name + "_binary_inverse.png", inverted);  // 保存二值化图像

    // 创建与图像相同尺寸的矩阵
    for (int A = 0; A < height; ++A) {
        for (int B = 0; B < width; ++B) {
            matrix.at<uchar>(A, B) = inverted.at<uchar>(A, B);
        }
    }
    return true;
}


//dfs搜索：寻找像素为pixel的连通区域，将其坐标存入cluster
void dfs(int y, int z, Mat & matrix, vector<vector<bool>>& visited, vector<Point>& cluster,int pixel) {
    int C = matrix.rows;
    int D = matrix.cols;

    vector<Point> tempCluster = { {y, z} };
    visited[y][z] = true;
    vector<Point> directions = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

    while (!tempCluster.empty()) {
        Point current = tempCluster.back();
        tempCluster.pop_back();
        cluster.push_back(current);

        for (const auto& dir : directions) {
            int ny = current.x + dir.x;
            int nz = current.y + dir.y;
            if (ny >= 0 && ny < C && nz >= 0 && nz < D && !visited[ny][nz] && matrix.at<uchar>(ny, nz) == pixel) {
                tempCluster.push_back({ ny, nz });
                visited[ny][nz] = true;
            }
        }
    }
}

//采用循环遍历方法，调用dfs，找到所有像素值为pixel(=0,255)的连通区域，存储再clusters中：
vector<vector<Point>> findClusters(Mat& matrix,int pixel) {
    int C = matrix.rows;
    int D = matrix.cols;
    vector<vector<bool>> visited(C, vector<bool>(D, false));
    vector<vector<Point>> clusters;

    for (int y = 0; y < C; ++y) {
        for (int z = 0; z < D; ++z) {
            if (matrix.at<uchar>(y, z) == pixel && !visited[y][z]) {
                vector<Point> cluster;
                dfs(y, z, matrix, visited, cluster, pixel);
                if (!cluster.empty()) {
                    clusters.push_back(cluster);
                }
            }
        }
    }
    return clusters;
}

//深拷贝
vector<vector<Point>> deepcopyClusters(vector<vector<Point>>& clusters) {
    vector<vector<Point>> new_clusters;

    // 遍历原始 clusters 进行深拷贝
    for (const auto& cluster : clusters) {
        new_clusters.emplace_back(cluster); // 使用拷贝构造函数
    }

    return new_clusters; // 返回深拷贝后的新矩阵
}

vector<double> processClusters(vector<vector<Point>>& clusters, const string& name, int height, int width,int pixel) {
    std::vector<double> new_list;
    cout << "Cluster Num:" << clusters.size() << endl;

    int NUM = 0;
    for (; NUM < clusters.size(); ++NUM) {
        if (clusters[NUM].size() > 1) {
            cv::Mat img_single_rgb = cv::Mat::zeros(width, height, CV_8UC3);
            int F = clusters[NUM].size();
            cout << "Num:" << NUM << ", F:" << F << endl;

            for (auto& point : clusters[NUM]) {
                img_single_rgb.at<cv::Vec3b>(point) = cv::Vec3b(255, 255, 255);
            }

            //单个初分割聚类图
            //string path1 = resultFolder + "\\pre\\cluster\\" + name + "_cluster_" + std::to_string(NUM) + "_cnt_" + std::to_string(F) + ".png";
            //cout << path1 << endl;
            cv::imwrite(resultFolder + "\\pre\\cluster_" + to_string(pixel)+"\\" + name + "_cluster_" + to_string(NUM) + "_cnt_" + to_string(F) + ".png", img_single_rgb);
            cv::Mat gray;
            cv::cvtColor(img_single_rgb, gray, cv::COLOR_BGR2GRAY);

            vector<vector<Point>> contours;
            cv::findContours(gray, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::drawContours(img_single_rgb, contours, -1, cv::Scalar(0, 0, 255), 2);

            //轮廓图
            cv::imwrite(resultFolder + "\\pre\\contour_" + to_string(pixel) + "\\" + name + "_cluster_" + to_string(NUM) + "_cnt_" + to_string(F) + ".png", img_single_rgb);
            float L = cv::arcLength(contours[0], true);
            new_list.push_back(L);
        }
    }
    return new_list;

}


double calculate_length_of_contour(const Mat& img) {
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(gray, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    if (!contours.empty()) {
        return arcLength(contours[0], true);
    }
    return 0.0;
}

void Runner::start(const QStringList& inputs) {

    if (inputs.size() < 4) {
        qWarning() << "Insufficient input values.";
        return;
    }

    // 将输入数据转为浮点数并存储
    bool ok;
    resolution = inputs[0].toDouble(&ok);
    if (!ok) {
        qWarning() << "Invalid resolution value:" << inputs[0];
    }

    threshold = inputs[1].toDouble(&ok);
    if (!ok) {
        qWarning() << "Invalid threshold value:" << inputs[1];
    }

    ssim = inputs[2].toDouble(&ok);
    if (!ok) {
        qWarning() << "Invalid ssim value:" << inputs[2];
    }

    iteration_num = inputs[3].toInt(&ok);
    if (!ok) {
        qWarning() << "Invalid iternum value:" << inputs[3];
    }

    QString result = performTask();
    emit finished(result); // 发射信号，告知 QML 任务完成
}
