// c++
#include <iostream>
#include <string.h>
// opencv
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    // check input 
    if (argc != 2) {
        cout << "Input failed. Please input ./PixelSelector ./pathToImage/test.png..." << endl;
        return -1;
    }

    // read image 
    string imgPath = argv[1];
    Mat image = imread(imgPath, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        cout << "Failed to read image, please check your path in : \n" << imgPath << endl;
        return -1;
    }

    // display image 
    imshow("InputImage", image);
    waitKey(0);

    return 0;
}