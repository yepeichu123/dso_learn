// c++
#include <iostream>
#include <string.h>
// opencv
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
// self define
#include "Frame.h"
#include "PixelSelector.h"

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

    // compute pyramid image 
    shared_ptr<DSOLearn::Frame> frame = make_shared<DSOLearn::Frame>(image);
    frame->displayImageLevel(1);
    // frame->displayImageAllLevel();

    // initialize pixel selector
    int width = frame->mvnImgWidth_[0];
    int height = frame->mvnImgHeight_[0];
    shared_ptr<DSOLearn::PixelSelector> pixelSelector = make_shared<DSOLearn::PixelSelector>(width, height);

    // select pixels
    float* outputMap = new float[width * height];
    int numPixelSelected = pixelSelector->makeMaps(frame, outputMap, 2000);
    cout << "We select " << numPixelSelected << " pixels in image." << endl;

    // display the selected pixels
    pixelSelector->displayPixelsSelected(frame, outputMap);


    delete []outputMap;
    return 0;
}