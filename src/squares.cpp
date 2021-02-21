#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;
int thresh = 50, N = 11;

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle(Point pt1, Point pt2, Point pt0)
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// returns sequence of squares detected on the image.
static void findSquares(Mat& image, vector<vector<Point> >& squares)
{
    squares.clear();
    Mat pyr, timg, gray0(image.size(), CV_8U), gray;
    // down-scale and upscale the image to filter out the noise
    pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
    pyrUp(pyr, timg, image.size());
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
        // find squares in every color plane of the image
    for( int c = 0; c < 3; c++ )
    {
        int ch[] = {c, 0};
        mixChannels(&timg, 1, &gray0, 1, ch, 1);
        // try several threshold levels
        for( int l = 0; l < N; l++ )
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
            if( l == 0 )
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                Canny(gray0, gray, 0, thresh, 5);
                // dilate canny output to remove potential
                // holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = gray0 >= (l+1)*255/N;
            }

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
                    if(maxCosine < 0.1 && hierarchy[i][3] >= 0)

                        // Try to detect if the square in question is not bounded
                        // within another square
                        if (squares.size() == 0) {
                            squares.push_back(approx);
                        } else {
                            int sqrSz = squares.size();
                            for (int k = 0; k < sqrSz; k++) {
                                vector<Point> square = squares.at(k);
                                cout << abs(square.at(0).x - approx.at(0).x) << " " << abs(square.at(0).y - approx.at(0).y) << "\n";
                                if (abs(square.at(0).x - approx.at(0).x > 10) && abs(square.at(0).y - approx.at(0).y > 10)) {
                                    squares.push_back(approx);
                                    sqrSz++;    
                                }
                                // cout << approx.at(0).x << " " << approx.at(0).y << "\n";
                                // cout << approx.at(1).x << " " << approx.at(1).y << "\n";
                                // cout << approx.at(2).x << " " << approx.at(2).y << "\n";
                                // cout << approx.at(3).x << " " << approx.at(3).y << "\n---\n";
                                // squares.push_back(approx);
                            }
                        }
                }
            }
        }   
    }
}

// the function draws all the squares in the image
static void drawSquares(Mat& image, const vector<vector<Point>>& squares)
{
    for(size_t i = 0; i < squares.size(); i++)
    {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, Scalar(0,200,0), 9, LINE_AA);
    }
    imwrite("output.jpg", image);
}

static void writeCoords(const vector<vector<Point>>& squares, string outPath, double scale) {
    ofstream cmdout;
    cmdout.open(outPath);
    
    for (vector<Point> square : squares) {
        // s logcabin1@20,10,12
        // in CV --->  x     y        
        string building = "logcabin1";
        cmdout << "s " << building << "@" << (int) (square.at(0).x * scale) << ",0," << (int) (square.at(0).y * scale) << "\n";
    }

    cmdout.close();
}

static void diffSquares(
    Mat& image,
    const vector<vector<Point>>& squares,
    vector<vector<Point>>& blackSquares,
    vector<vector<Point>>& redSquares,
    vector<vector<Point>>& blueSquares) {


    // Check for red value above a certain threshold.
    // Mat channel[3];
    // split(image, channel);
    // channel[0]=Mat::zeros(image.rows, image.cols, CV_8UC1);//Set blue channel to 0
    // channel[1]=Mat::zeros(image.rows, image.cols, CV_8UC1);//Set green channel to 0
    // // channel[2]=Mat::zeros(image.rows, image.cols, CV_8UC1);//Set red channel to 0

    // merge(channel,3,image);

    blur( image, image, Size(50, 50)); 
    imwrite("tmp02.jpg", image);
    
    for (vector<Point> square : squares) {
        Point checkPoint = { square.at(0).x, square.at(0).y };
        int offset = 5;
        int blue = image.at<cv::Vec3b>(checkPoint.y + offset,checkPoint.x + offset)[0];
        int green = image.at<cv::Vec3b>(checkPoint.y + offset,checkPoint.x + offset)[1];
        int red = image.at<cv::Vec3b>(checkPoint.y + offset,checkPoint.x + offset)[2];
        // Vec3b bgrPixel = image.at<Vec3b>(, );
        cout << blue << " " << green << " " << red;
        if (blue < 150 && green < 150 && red > 150) {
            std::cout << " - Red!\n";
            redSquares.push_back(square);
        } else if (blue > 150 && green < 150 && red < 150) {
            std::cout << " - Blue!\n";
            blueSquares.push_back(square);
        } else {
            std::cout << " - Black!\n";
            blackSquares.push_back(square);
        }
    }

}

// static void findFills(Mat& image/*, vector<vector<Point>>& fillSquares*/)
// {
//     Mat channel[3];
//     split(image, channel);
//     channel[0]=Mat::zeros(image.rows, image.cols, CV_8UC1);//Set blue channel to 0
//     channel[1]=Mat::zeros(image.rows, image.cols, CV_8UC1);//Set green channel to 0
//     channel[2]=Mat::zeros(image.rows, image.cols, CV_8UC1);//Set red channel to 0

//     merge(channel,3,image);
    
//     vector<vector<Point>> bricks;

//     findSquares(image, bricks);
//     drawSquares(image, bricks);
// }

int main(int argc, char** argv)
{
    // For the squares we initially get
    vector<vector<Point>> squares;

    // For the types of squares
    vector<vector<Point>> blackSquares;
    vector<vector<Point>> redSquares;
    vector<vector<Point>> blueSquares;

    string filename = samples::findFile(argv[1]);
    Mat image = imread(filename, IMREAD_COLOR);
    if(image.empty())
        cout << "Couldn't load " << filename << endl;
        
    findSquares(image, squares);
    // diffSquares(image, squares, blackSquares, redSquares, blueSquares);
    drawSquares(image, squares);
    // writeCoords(squares, argv[2], 0.1);
    // findFills(image);
    return 0;
}