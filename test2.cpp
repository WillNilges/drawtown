#include <opencv2/opencv.hpp>


int main() {    
    cv::Mat img = cv::imread("tiddy2.jpg");   // Read the file
    double threshold = 60; // needs adjustment.
    int n_erode_dilate = 1; // needs adjustment.

    cv::Mat m = img.clone();
    cv::cvtColor(m, m, cv::COLOR_RGB2GRAY); // convert to grayscale image.
    cv::blur(m, m, cv::Size(5,5));
    cv::threshold(m, m, threshold, 255, cv::THRESH_BINARY_INV);
    imwrite("tmp1.jpg", m);
    cv::erode(m, m, cv::Mat(),cv::Point(-1,-1),n_erode_dilate);
    cv::dilate(m, m, cv::Mat(),cv::Point(-1,-1),n_erode_dilate);

    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Point> points;
    cv::findContours(m, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    for (size_t i=0; i<contours.size(); i++) {
        for (size_t j = 0; j < contours[i].size(); j++) {
            cv::Point p = contours[i][j];
            points.push_back(p);
        }
    }
    // And process the points or contours to pick up specified object.



    for (std::vector<cv::Point> contour : contours){
        // Get the stright bounding rect
        cv::Rect bbox = cv::boundingRect(contour);
        int x, y, w, h;
        x = bbox.x;
        y = bbox.y;
        w = bbox.width;
        h = bbox.height;

        int thresh = 200;
        if (w > thresh || h > thresh)
            cv::rectangle(img, bbox.tl(), bbox.br(), cv::Scalar(0, 200, 0), 8, cv::LINE_AA);
    }

    imwrite("tmp2.jpg", img);

}