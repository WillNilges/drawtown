#include "squares.h"
int thresh = 50, N = 11;

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
double angle(Point pt1, Point pt2, Point pt0)
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

namespace DrawTown {
// returns sequence of squares detected on the image.
void findSquares(const Mat& image, vector<vector<Point>>& squares)
{
    squares.clear();
    Mat pyr, timg, gray0(image.size(), CV_8U), gray;
    // down-scale and upscale the image to filter out the noise
    pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
    pyrUp(pyr, timg, image.size());
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    int c = 0;
    int ch[] = {c, 0};
    mixChannels(&timg, 1, &gray0, 1, ch, 1);
    gray = gray0 >= (4+1)*255/N;
    // find contours and store them all as a list
    findContours(gray, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

    vector<Point> approx;
    // test each contour
    for(size_t i = 0; i < contours.size(); i++)
    {
        // approximate contour with accuracy proportional
        // to the contour perimeter
        approxPolyDP(contours[i], approx, arcLength(contours[i], true)*0.02, true);
        // square contours should have 4 vertices after approximation
        // relatively large area (to filter out noisy contours)
        // and be convex.
        // Note: absolute value of an area is used because
        // area may be positive or negative - in accordance with the
        // contour orientation
        if(approx.size() == 4 &&
            fabs(contourArea(approx)) > 1000 &&
            isContourConvex(approx))
        {
            double maxCosine = 0;
            for(int j = 2; j < 5; j++)
            {
                // find the maximum cosine of the angle between joint edges
                double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                maxCosine = MAX(maxCosine, cosine);
            }
            // if cosines of all angles are small
            // (all angles are ~90 degree) then write quandrange
            // vertices to resultant sequence
            if(maxCosine < 0.3 && hierarchy[i][3] >= 0)
                    squares.push_back(approx);
        }
    }
}

void findCircles(const Mat& image, vector<Vec3f>& circles)
{
    Mat gray;
    cvtColor(image, gray, COLOR_BGR2GRAY);
    medianBlur(gray, gray, 5);
    // vector<Vec3f> circles;
    HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
                 gray.rows/12,  // change this value to detect circles with different distances to each other
                 100, 30, 30, 200 // change the last two parameters
            // (min_radius & max_radius) to detect larger circles
    );
}

void drawCircles(Mat& image, vector<Vec3f>& circles)
{
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        // circle center
        circle( image, center, 1, Scalar(0,100,100), 3, LINE_AA);
        // circle outline
        int radius = c[2];
        circle( image, center, radius, Scalar(255,0,255), 3, LINE_AA);
    }    
}

// the function draws all the squares in the image
void drawSquares(Mat& image, const vector<vector<Point>>& squares)
{
    for(size_t i = 0; i < squares.size(); i++)
    {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, Scalar(0,200,0), 9, LINE_AA);
    }
}

void out(const Mat& image) {
    imwrite("result.jpg", image);
}

void writeCoords(
    const Mat& image,
    const vector<vector<Point>>& squares,
    const vector<Vec3f>& circles,
    string outPath,
    double scale)
{
    ofstream cmdout;
    cmdout.open(outPath);

    cmdout << "r default:dirt_with_grass@0,0 " << (int) (image.cols * scale) << "," << (int) (image.rows * scale) << "\n";
    
    for (vector<Point> square : squares) {
        // s logcabin1@20,10,12
        // in CV --->  x     y    
        int build = square.at(0).x % 2;
        string building;
        switch (build) {
            case 0:
                building = "tower";
                break;
            default:
                building = "cabin";
        }
        cmdout << "s " << building << "@" << (int) (square.at(0).x * scale) << "," << (int) (square.at(0).y * scale) << "\n";
    }

    for (Vec3f circle : circles) {
        string building = "forest";
        cmdout << "s " << building << "@" << (int) (circle[0] * scale) << "," << (int) (circle[1] * scale) << "\n";
    }

    cmdout << "\n";

    cmdout.close();
}
}
