#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
// #include "opencv2/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

namespace DrawTown
{

void findSquares(const Mat& image, vector<vector<Point>>& squares);

void findCircles(const Mat& image, vector<Vec3f>& circles);

void drawCircles(Mat& image, vector<Vec3f>& circles);

void drawSquares(Mat& image, const vector<vector<Point>>& squares);

void out(const Mat& image);

void writeCoords(
    const Mat& image,
    const vector<vector<Point>>& squares,
    const vector<Vec3f>& circles,
    string outPath
);

}
