// kmeans.h
#ifndef KMEANS_H
#define KMEANS_H

#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <filesystem>

#include<QString>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

double calculateDistance(const Point& a, const Point& b);

bool reSplit(vector<vector<Point>>& clusters, double threshold, int circle_count, double max_x, double ssim, double resolution,
             int height, int width, string resultFolder, string name,
             vector<int>* repair_klist, vector<double>* diameter_list, vector<double>* percentage_list, bool revised);

bool ClusterDivide(int clusterID, int split_number, vector<Point>& cluster, double resolution, double threshold, double ssim, double max_x, int circle_count,
                   int height, int width, string resultFolder, string name, vector<int>* repair_klist, vector<double>* diameter_list, vector<double>* percentage_list, bool revised);

bool compareWithSSIM(vector<vector<Point>> centers_list, vector<vector<vector<Point>>> clusters_list_copied,
                     int clusterID, int iter_num, int split_number, double ssim, double resolution, int height, int width, string resultFolder, string name,
                     vector<int>* repair_klist, vector<double>* diameter_list, vector<double>* percentage_list, bool revised);
vector<vector<vector<Point>>> deepcopyClustersList(vector<vector<vector<Point>>>& clusters_list);
vector<QString> parallel_second_repair(vector<int> repair_second_klist, vector<vector<Point>> filtered_clusters,
    double max_x,vector<vector<Point>> extractedClusters_2,int height,int width,string resultFolder,string name,
    vector<int> redundant_list,vector<double> diameter_list,vector<double> percentage_list,
    double threshold,int iteration_num,double resolution,double ssim,int circle_count);
    
#endif // KMEANS_H
