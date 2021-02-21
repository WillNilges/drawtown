#include "squares.h"

using namespace DrawTown;

int main(int argc, char** argv)
{
    vector<vector<Point>> squares;

    vector<Vec3f> circles;

    string filename = samples::findFile(argv[1]);
    Mat image = imread(filename, IMREAD_COLOR);
    if(image.empty())
        cout << "Couldn't load " << filename << endl;

    findSquares(image, squares);
    findCircles(image, circles);
    drawSquares(image, squares);
    drawCircles(image, circles);
    writeCoords(squares, circles, argv[2], 0.1);
    
    return 0;
}