// visualize.h
#ifndef VISUALIZE_H
#define VISUALIZE_H

#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <filesystem>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

// 处理 SSIM 计算
struct SSIMResult {
    double score;
    cv::Mat diff;
};

vector<vector<Point>> visualize(vector<vector<Point>>& clusters, string resultFolder, string name, double resolution, int height, int width);
void saveHistogram(vector<vector<Point>>& clusters, string filename);
void visualizeClusters(const vector<vector<Point>>& clusters1, size_t count, int height, int width, string& resultFolder, const string& name);
double ouputSSIMResult(string beforePath, string afterPath, string resultFolder, string name, bool compare);
double calculateSSIM(const cv::Mat& img1, const cv::Mat& img2, Mat& diff);
double gauss(double x);
double get_kde(double x, vector<double>& data_array, double bandwidth);
double saveKde(vector<vector<Point>>& clusters);
vector<vector<Point>> deepcopyClusters(const vector<vector<Point>>& clusters);

#endif // VISUALIZE_H
