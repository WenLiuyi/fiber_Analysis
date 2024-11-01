#include <iostream>
#include <xlnt/xlnt.hpp>
#include <filesystem>
#include <fstream>
#include <numeric> // For std::accumulate
#include <cmath>   // For std::pow
#include <set>     // For std::set

#include "createDirectoriesExcels.h"
namespace fs = std::filesystem;

//创建各目录：
void createDirectories(const std::string& baseFolder) {    //baseFolder为根目录，name为原图的命名
    std::vector<std::string> subFolders = {
        "", "/pre", "/final", "/pre/cluster_0","/pre/cluster_255","/pre/contour_0","/pre/contour_255","/pre/pixel inverse","/pre/histogram",
        "/pre/dyed","/pre/segment","/pre/dyed_segment","/pre/processed"
        "/final/cir", "/final/cluster", "/final/segment", "/final/segment contour", "/final/segment contour/ellipse fit", "/final/result"
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

void save_diameter_info(string filePath, string excelPath, vector<double> diameter_list, vector<int> repair_klist) {
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
    for (size_t i = 0; i < diameter_list.size(); i++) {
        // 对单丝的序号数遍历
        vector<pair<string, double>> testData = {{"cluster_"+ to_string(*next(list_dianum.begin(), i)),diameter_list[i]}};
        diameters_to_Excel(excelPath, testData, i);
    }
}

double calculateVariance(const vector<double>& data, double mean) {
    double variance = 0.0;
    for (const double& value : data) {
        variance += std::pow(value - mean, 2);
    }
    return variance / data.size();
}

void diameters_to_Excel(string excelPath,vector<pair<string, double>>& data, int i) {
    xlnt::workbook wb;

    // 尝试打开一个现有的 Excel 文件
    try {
        wb.load(excelPath); // 请确保有一个 example.xlsx 文件存在
    }catch (const std::exception& e) {
        std::cerr << "Error loading workbook: " << e.what() << std::endl;
        return;
    }

    // 获取工作表
    auto ws = wb.active_sheet();

    // 写入新数据
    for (size_t i = 0; i < data.size(); i++) {
        ws.cell(1, i + 2).value(data[i].first);
        ws.cell(2, i + 2).value(to_string(data[i].second));
    }
    // 保存修改后的工作簿
    try {
        wb.save("example_updated.xlsx"); // 保存为新的文件
    }catch (const std::exception& e) {
        std::cerr << "Error saving workbook: " << e.what() << std::endl;
        return;
    }
    wb.save(excelPath);
}
