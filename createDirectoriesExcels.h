// createDirectoriesExcels.h
#ifndef CREATEDIRECTORIESEXCELS_H
#define CREATEDIRECTORIESEXCELS_H

#include <iostream>
#include<vector>

#include <wchar.h>

using namespace std;

void createDirectories(const std::string& baseFolder);             //创建目录
void createExcelFile(const std::string& filePath, const std::vector<std::string>& titles);  //创建Excel表格

void save_diameter_info(string filePath, string excelPath, vector<double> diameter_list, vector<int> repair_klist);
double calculateVariance(const std::vector<double>& data, double mean);
void diameters_to_Excel(string excelPath, vector<pair<string, double>>& data, int i);

#endif // CREATEDIRECTORIESEXCELS_H
