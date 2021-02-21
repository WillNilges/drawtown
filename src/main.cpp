#include "squares.h"

int main(int argc, char** argv)
{
    vector<vector<Point>> squares;

    vector<Vec3f> circles;

    string filename = samples::findFile(argv[1]);
    Mat image = imread(filename, IMREAD_COLOR);
    if(image.empty())
        cout << "Couldn't load " << filename << endl;

    DrawTown::findSquares(image, squares);
    DrawTown::findCircles(image, circles);
    DrawTown::drawSquares(image, squares);
    DrawTown::drawCircles(image, circles);
    DrawTown::writeCoords(squares, circles, argv[2], 0.1);

    return 0;
}
