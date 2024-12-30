#include <iostream>
#include <xlnt/xlnt.hpp>
#include <filesystem>
#include <fstream>
#include <numeric> // For std::accumulate
#include <cmath>   // For std::pow
#include <set>     // For std::set
#include <regex>

#include <opencv.hpp>

#include<matplot/matplot.h>

#include "createDirectoriesExcels.h"
#include "visualize.h"
namespace fs = std::filesystem;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//创建各目录：
void createDirectories(const std::string& baseFolder) {    //baseFolder为根目录，name为原图的命名
    std::vector<std::string> subFolders = {
        "", "/pre", "/final", "/pre/cluster_0","/pre/cluster_255","/pre/contour_0","/pre/contour_255","/pre/pixel inverse","/pre/histogram",
        "/pre/dyed","/pre/segment","/pre/dyed_segment","/pre/processed",
        "/final/cir", "/final/cluster", "/final/segment", "/final/segment contour", "/final/segment contour/ellipse fit", "/final/result",
        "/final/diameter texts" ,"/final/diameter images", "/final/roundness images", "/final/roundness texts"
    };

    for (const auto& folder : subFolders) {
        fs::create_directories(baseFolder + folder);
    }
}

//创建Excel表格：
void createExcelFile(const std::string& filePath, const std::vector<std::string>& titles) { //filePath为表格名，titles为表头
    xlnt::workbook wb;
    xlnt::worksheet ws = wb.active_sheet();        //创建子表

    try {
        for (size_t i = 0; i < titles.size(); ++i) {
            //std::cout << "Setting cell (1, " << i + 1 << ") to: " << titles[i] << std::endl;
            ws.cell(i + 1, 1).value(titles[i]);
        }
        wb.save(filePath);
    }
    catch (const xlnt::exception& e) {
        std::cerr << "xlnt Exception: " << e.what() << std::endl;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Out of range error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

}

void xw_toExcel(std::vector<std::map<std::string, std::string>>& testData, int cnt) {
    xlnt::workbook wb;
    xlnt::worksheet ws = wb.create_sheet();
    ws.title("Fiber Roundness");

    // Set header row
    ws.cell("A1").value("TP");
    ws.cell("B1").value("SR");
    ws.cell("C1").value("Ellipse Ratio");
    ws.cell("D1").value("Distance");
    ws.cell("E1").value("Short Ratio");
    ws.cell("F1").value("Long Ratio");

    int row = 2;
    for (const auto& data : testData) {
        ws.cell("A" + std::to_string(row)).value(data.at("TP"));
        ws.cell("B" + std::to_string(row)).value(std::stof(data.at("SR")));
        ws.cell("C" + std::to_string(row)).value(std::stof(data.at("ellipse ratio")));
        ws.cell("D" + std::to_string(row)).value(std::stof(data.at("distance")));
        ws.cell("E" + std::to_string(row)).value(std::stof(data.at("short ratio")));
        ws.cell("F" + std::to_string(row)).value(std::stof(data.at("long ratio")));
        row++;
    }

    wb.save("fiber_roundness_data.xlsx");
}

void save_roundness_info(string name, string roundness_filePath, string roundness_excelPath, string resultFolder, string folderPath) {
    std::ofstream file(roundness_filePath);
    file << "各聚类的圆度表征参量:";

    std::vector<double> section_round_list, section_round1_list, section_round2_list, section_round3_list, section_round4_list;
    std::regex clusterIDPattern(name + "_cluster_(\\d+)\\.png");

    int cnt = 0;


    xlnt::workbook wb;

    // 尝试打开一个现有的 Excel 文件
    try {
        wb.load(roundness_excelPath); 
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading workbook: " << e.what() << std::endl;
        return;
    }

    // 获取名为 name 的工作表，如果没有则创建
    xlnt::worksheet ws;
    try {
        ws = wb.sheet_by_title(name); // 尝试加载名为 name 的工作表
    }
    catch (const std::exception& e) {
        // 如果没有名为 name 的工作表，则创建一个新工作表
        std::cerr << "Worksheet 'file' not found. Creating a new one." << std::endl;
        ws = wb.create_sheet(); // 创建一个新的工作表
        ws.title(name); // 设置工作表名称为 name
    }

    try {
        const std::vector<std::string> titles = { "Cluster Type", "S/L", "a/b", "d/D", "2R/A", "2R/B" };
        for (size_t i = 0; i < titles.size(); ++i) {
            ws.cell(i + 1, 1).value(titles[i]);
        }
    }
    catch (const xlnt::exception& e) {
        std::cerr << "xlnt Exception: " << e.what() << std::endl;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Out of range error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        std::string filename = entry.path().filename().string();
        std::smatch match;

        if (std::regex_match(filename, match, clusterIDPattern)) {
            int clusterID = std::stoi(match[1].str());
            if (filename.find("_circle_highlighted") == std::string::npos
                && filename.find("_circled") == std::string::npos) {

                cv::Mat section_pic = cv::imread(entry.path().string());
                if (section_pic.empty()) {
                    std::cerr << "Error reading image: " << entry.path() << std::endl;
                    continue;
                }

                int height = section_pic.rows, width = section_pic.cols;

                // 1. 灰度化
                cv::Mat gray;
                cv::cvtColor(section_pic, gray, cv::COLOR_BGR2GRAY);

                // 2. 提取图像外轮廓
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(gray, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                // 3. 在分割的单丝上绘制外轮廓
                cv::drawContours(section_pic, contours, -1, cv::Scalar(0, 0, 255), 2);
                cv::imwrite(resultFolder + "\\final\\segment contour\\" + name +".png", section_pic);

                // 4. 单丝最大轮廓周长
                double max_perimeter = 0.0;
                int max_index = -1;
                for (int i = 0; i < contours.size(); ++i) {
                    double perimeter = cv::arcLength(contours[i], true);
                    if (perimeter > max_perimeter) {
                        max_perimeter = perimeter;
                        max_index = i;
                    }
                }

                // 5. 对应面积
                double area = cv::contourArea(contours[max_index]);
                double perimeter = max_perimeter;

                // 6. 单丝面积周长特征比
                double section_round = (4 * M_PI * area) / std::pow(perimeter, 2);
                section_round_list.push_back(section_round);

                // 7. 计算拟合椭圆参数
                cv::RotatedRect ellipse = cv::fitEllipse(contours[max_index]);
                double axis_ratio = ellipse.size.width / ellipse.size.height;
                section_round1_list.push_back(axis_ratio);

                // 8. 计算像素点到轮廓的距离
                cv::Mat raw_dist = cv::Mat::zeros(gray.size(), CV_32F);
                for (int i = 0; i < height; ++i) {
                    for (int j = 0; j < width; ++j) {
                        raw_dist.at<float>(i, j) = cv::pointPolygonTest(contours[max_index], cv::Point(j, i), true);
                    }
                }

                // 9. 获取最大值即内接圆半径，中心点坐标
                double minVal, maxVal;
                cv::Point maxDistPt;
                cv::minMaxLoc(raw_dist, &minVal, &maxVal, nullptr, &maxDistPt);
                double maxRadius = std::abs(maxVal);
                double deviate_distance_dimles = cv::norm(maxDistPt - cv::Point(ellipse.center)) / (2 * std::sqrt(area / M_PI));

                // 10.
                double min_edge_length = min(ellipse.size.width, ellipse.size.height);
                double max_edge_length = max(ellipse.size.width, ellipse.size.height);
                double incision_diameter = 2 * maxRadius;
                double min_diameter_ratio_edge = incision_diameter / min_edge_length;
                double max_diameter_ratio_edge = incision_diameter / max_edge_length;

                section_round2_list.push_back(deviate_distance_dimles);
                section_round3_list.push_back(min_diameter_ratio_edge);
                section_round4_list.push_back(max_diameter_ratio_edge);

                file << "\n" << "cluster_" << clusterID << "对应单丝的面积周长特征比为" << section_round;
                file << "\n" << "cluster_" << clusterID << "对应拟合椭圆的短长半轴比为" << axis_ratio;
                file << "\n" << "cluster_" << clusterID << "单丝相对偏心距为" << deviate_distance_dimles;
                file << "\n" << "cluster_" << clusterID << "单丝直径短边比为" << min_diameter_ratio_edge;
                file << "\n" << "cluster_" << clusterID << "单丝直径长边比为" << max_diameter_ratio_edge;

                std::vector<std::map<std::string, std::string>> testData = {
                    {{"TP", name}, {"SR", std::to_string(section_round)}, {"ellipse ratio", std::to_string(axis_ratio)}, {"distance", std::to_string(deviate_distance_dimles)},
                     {"short ratio", std::to_string(min_diameter_ratio_edge)}, {"long ratio", std::to_string(max_diameter_ratio_edge)}}
                };
                
                ws.cell(1, cnt + 2).value("cluster_"+to_string(clusterID));
                ws.cell(2, cnt + 2).value(section_round);
                ws.cell(3, cnt + 2).value(axis_ratio);
                ws.cell(4, cnt + 2).value(deviate_distance_dimles);
                ws.cell(5, cnt + 2).value(min_diameter_ratio_edge);
                ws.cell(6, cnt + 2).value(max_diameter_ratio_edge);

                cnt++;
            }
        }
    }
    try {
        wb.save(roundness_excelPath); // 保存为新的文件
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving workbook: " << e.what() << std::endl;
        return;
    }

    // Write final statistics to file
    file << "\n单丝面积周长特征比均值为" << std::accumulate(section_round_list.begin(), section_round_list.end(), 0.0) / section_round_list.size();
    file << "\n单丝短长半轴之比均值为" << std::accumulate(section_round1_list.begin(), section_round1_list.end(), 0.0) / section_round1_list.size();
    file << "\n单丝相对偏心距均值为" << std::accumulate(section_round2_list.begin(), section_round2_list.end(), 0.0) / section_round2_list.size();
    file << "\n单丝直径短边比均值为" << std::accumulate(section_round3_list.begin(), section_round3_list.end(), 0.0) / section_round3_list.size();
    file << "\n单丝直径长边比均值为" << std::accumulate(section_round4_list.begin(), section_round4_list.end(), 0.0) / section_round4_list.size();

    file.close();

    // Plot and save histograms using matplot++
    double min_value = *std::min_element(section_round_list.begin(), section_round_list.end());
    double max_value = *std::max_element(section_round_list.begin(), section_round_list.end());

    int n_bins = 20;
    double bin_width = (max_value - min_value) / n_bins;

    matplot::hist(section_round_list, n_bins);
    matplot::xlabel("单丝面积周长特征比");
    matplot::ylabel("单丝数目");
    matplot::xlim({ min_value, max_value });
    matplot::save(convertPath(resultFolder + "\\final\\roundness images\\" + name + " Area-Perimeter_Ratio.png"));


    min_value = *std::min_element(section_round1_list.begin(), section_round1_list.end());
    max_value = *std::max_element(section_round1_list.begin(), section_round1_list.end());
    bin_width = (max_value - min_value) / n_bins;
    matplot::hist(section_round1_list, n_bins);
    matplot::xlabel("单丝短长半轴之比");
    matplot::ylabel("单丝数目");
    matplot::xlim({ min_value, max_value });
    matplot::save(convertPath(resultFolder + "\\final\\roundness images\\" + name + " Short-to-Long_Axis_Ratio.png"));


    min_value = *std::min_element(section_round2_list.begin(), section_round2_list.end());
    max_value = *std::max_element(section_round2_list.begin(), section_round2_list.end());
    bin_width = (max_value - min_value) / n_bins;
    matplot::hist(section_round2_list, n_bins);
    matplot::xlabel("单丝相对偏心距");
    matplot::ylabel("单丝数目");
    matplot::xlim({ min_value, max_value });
    matplot::save(convertPath(resultFolder + "\\final\\roundness images\\" + name + " Relative_Eccentricity.png"));


    min_value = *std::min_element(section_round3_list.begin(), section_round3_list.end());
    max_value = *std::max_element(section_round3_list.begin(), section_round3_list.end());
    bin_width = (max_value - min_value) / n_bins;
    matplot::hist(section_round3_list, n_bins);
    matplot::xlabel("单丝直径短边比");
    matplot::ylabel("单丝数目");
    matplot::xlim({ min_value, max_value });
    matplot::save(convertPath(resultFolder + "\\final\\roundness images\\" + name + " Diameter_Short-to-Side_Ratio.png"));


    min_value = *std::min_element(section_round4_list.begin(), section_round4_list.end());
    max_value = *std::max_element(section_round4_list.begin(), section_round4_list.end());
    bin_width = (max_value - min_value) / n_bins;
    matplot::hist(section_round4_list, n_bins);
    matplot::xlabel("单丝直径长边比");
    matplot::ylabel("单丝数目");
    matplot::xlim({ min_value, max_value });
    matplot::save(convertPath(resultFolder + "\\final\\roundness images\\" + name + " Diameter_Long-to-Side_Ratio.png"));


    std::cout << "完成截面圆度参数计算" << std::endl;
}


void save_diameter_info(string name, string filePath, string excelPath, string imgPath, vector<double> diameter_list, vector<int> repair_klist) {
    ofstream diameter_file(filePath);

    // 1. 创建txt文件
    if (diameter_file.is_open()) {
        diameter_file << "最终分割拟合单丝的等效直径信息：";
        diameter_file.close();
    }else {
        cerr << "无法打开文件进行写入: " << filePath << endl; return;
    }

    // 2. 将直径信息写入文件
    diameter_file.open(filePath, ios::app);
    if (diameter_file.is_open()) {
        for (const double& diameter : diameter_list) {
            diameter_file << "\n" << diameter; // Write each diameter
        }
    }else {
        cerr << "无法打开文件进行追加: " << filePath << endl; return;
    }
    diameter_file.close();

    // 3. 计算单丝个数，单丝等效直径均值、方差
    diameter_file.open(filePath, std::ios::app);
    if (diameter_file.is_open()) {
        size_t fiber_count = diameter_list.size();
        double mean = std::accumulate(diameter_list.begin(), diameter_list.end(), 0.0) / fiber_count;
        double variance = calculateVariance(diameter_list, mean);

        diameter_file << "\n单丝个数为" << fiber_count;
        diameter_file << "\n单丝等效直径均值为" << mean;
        diameter_file << "\n单丝等效直径方差为" << variance;
    }
    diameter_file.close();

    // 4. 输出至Excel表格
    set<int> list_dianum;       // 避免repair_klist的元素在diameter_list中重复出现
    for (size_t i = 0; i < diameter_list.size(); i++) {
        list_dianum.insert(i);
    }for (int k : repair_klist) {
        list_dianum.erase(k);
    }for (int k : repair_klist) {
        list_dianum.insert(k);
    }

    xlnt::workbook wb;

    // 尝试打开一个现有的 Excel 文件
    try {
        wb.load(excelPath); // 请确保有一个 example.xlsx 文件存在
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading workbook: " << e.what() << std::endl;
        return;
    }

    // 获取名为 name 的工作表，如果没有则创建
    xlnt::worksheet ws;
    try {
        ws = wb.sheet_by_title(name); // 尝试加载名为 name 的工作表
    }
    catch (const std::exception& e) {
        // 如果没有名为 name 的工作表，则创建一个新工作表
        std::cerr << "Worksheet 'file' not found. Creating a new one." << std::endl;
        ws = wb.create_sheet(); // 创建一个新的工作表
        ws.title(name); // 设置工作表名称为 name
    }

    try {
        const std::vector<std::string> titles = { "Cluster Type", "Equivalent Diameter(micrometer)" };
        for (size_t i = 0; i < titles.size(); ++i) {
            //std::cout << "Setting cell (1, " << i + 1 << ") to: " << titles[i] << std::endl;
            ws.cell(i + 1, 1).value(titles[i]);
        }
    }
    catch (const xlnt::exception& e) {
        std::cerr << "xlnt Exception: " << e.what() << std::endl;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Out of range error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    // 获取工作表
    //auto ws = wb.active_sheet();

    for (size_t i = 0; i < diameter_list.size(); i++) {
        // 对单丝的序号数遍历
        //vector<pair<string, double>> testData = {{"cluster_"+ to_string(*next(list_dianum.begin(), i)),diameter_list[i]}};

        pair<string, double > data =  {"cluster_" + to_string(*next(list_dianum.begin(), i)),diameter_list[i]} ;
        ws.cell(1, i + 2).value(data.first);
        ws.cell(2, i + 2).value(to_string(data.second));
    }
    // 保存修改后的工作簿
    try {
        wb.save(excelPath); // 保存为新的文件
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving workbook: " << e.what() << std::endl;
        return;
    }

    // 5. 输出单丝等效直径直方图
    double min_value = *std::min_element(diameter_list.begin(), diameter_list.end());
    double max_value = *std::max_element(diameter_list.begin(), diameter_list.end());

    int n_bins = 20;  
    double bin_width = (max_value - min_value) / n_bins;

    matplot::hist(diameter_list, n_bins);

    // Set x and y axis labels
    matplot::xlabel("单丝等效直径 (μm)");
    matplot::ylabel("单丝数目");

    matplot::xlim({ min_value, max_value });

    /*
    std::vector<double> xticks;
    double tick = min_value;
    int tick_interval = 5; // 每隔5个bin取一个tick
    while (tick <= max_value) {
        xticks.push_back(tick);
        tick += bin_width * tick_interval;  // 每次加上5个bin的宽度
    }
    matplot::xticks(xticks);
    */
    matplot::save(convertPath(imgPath));

}

double calculateVariance(const vector<double>& data, double mean) {
    double variance = 0.0;
    for (const double& value : data) {
        variance += std::pow(value - mean, 2);
    }
    return variance / data.size();
}
