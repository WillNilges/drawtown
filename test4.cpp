#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

Point2f computeIntersect(Vec2f line1, Vec2f line2);
vector<Point2f> lineToPointPair(Vec2f line);
bool acceptLinePair(Vec2f line1, Vec2f line2, float minTheta);

int main(int argc, char* argv[])
{
    Mat occludedSquare = imread("img/redrect01.jpg");

    resize(occludedSquare, occludedSquare, Size(0, 0), 0.25, 0.25);

    Mat occludedSquare8u;
    cvtColor(occludedSquare, occludedSquare8u, COLOR_BGR2GRAY);

    Mat thresh;
    threshold(occludedSquare8u, thresh, 200.0, 255.0, THRESH_BINARY);

    GaussianBlur(thresh, thresh, Size(7, 7), 2.0, 2.0);

    Mat edges;
    Canny(thresh, edges, 66.0, 133.0, 3);

    vector<Vec2f> lines;
    HoughLines( edges, lines, 1, CV_PI/180, 50, 0, 0 );

    cout << "Detected " << lines.size() << " lines." << endl;

    // compute the intersection from the lines detected...
    vector<Point2f> intersections;
    for( size_t i = 0; i < lines.size(); i++ )
    {
        for(size_t j = 0; j < lines.size(); j++)
        {
            Vec2f line1 = lines[i];
            Vec2f line2 = lines[j];
            if(acceptLinePair(line1, line2, CV_PI / 32))
            {
                Point2f intersection = computeIntersect(line1, line2);
                intersections.push_back(intersection);
            }
        }

    }

    if(intersections.size() > 0)
    {
        vector<Point2f>::iterator i;
        for(i = intersections.begin(); i != intersections.end(); ++i)
        {
            cout << "Intersection is " << i->x << ", " << i->y << endl;
            circle(occludedSquare, *i, 1, Scalar(0, 255, 0), 3);
        }
    }

    imshow("intersect", occludedSquare);
    waitKey();

    return 0;
}

bool acceptLinePair(Vec2f line1, Vec2f line2, float minTheta)
{
    float theta1 = line1[1], theta2 = line2[1];

    if(theta1 < minTheta)
    {
        theta1 += CV_PI; // dealing with 0 and 180 ambiguities...
    }

    if(theta2 < minTheta)
    {
        theta2 += CV_PI; // dealing with 0 and 180 ambiguities...
    }

    return abs(theta1 - theta2) > minTheta;
}

// the long nasty wikipedia line-intersection equation...bleh...
Point2f computeIntersect(Vec2f line1, Vec2f line2)
{
    vector<Point2f> p1 = lineToPointPair(line1);
    vector<Point2f> p2 = lineToPointPair(line2);

    float denom = (p1[0].x - p1[1].x)*(p2[0].y - p2[1].y) - (p1[0].y - p1[1].y)*(p2[0].x - p2[1].x);
    Point2f intersect(((p1[0].x*p1[1].y - p1[0].y*p1[1].x)*(p2[0].x - p2[1].x) -
                       (p1[0].x - p1[1].x)*(p2[0].x*p2[1].y - p2[0].y*p2[1].x)) / denom,
                      ((p1[0].x*p1[1].y - p1[0].y*p1[1].x)*(p2[0].y - p2[1].y) -
                       (p1[0].y - p1[1].y)*(p2[0].x*p2[1].y - p2[0].y*p2[1].x)) / denom);

    return intersect;
}

vector<Point2f> lineToPointPair(Vec2f line)
{
    vector<Point2f> points;

    float r = line[0], t = line[1];
    double cos_t = cos(t), sin_t = sin(t);
    double x0 = r*cos_t, y0 = r*sin_t;
    double alpha = 1000;

    points.push_back(Point2f(x0 + alpha*(-sin_t), y0 + alpha*cos_t));
    points.push_back(Point2f(x0 - alpha*(-sin_t), y0 - alpha*cos_t));

    return points;
}