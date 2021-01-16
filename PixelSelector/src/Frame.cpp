#include "Frame.h"
#include <iostream>
#include <opencv2/highgui.hpp>

using namespace DSOLearn;

// default constructor
Frame::Frame(const cv::Mat& image) {
    assert(!image.empty());
    mmImage_ = image.clone();

    // allocate memory of each container
    mvnImgWidth_.resize(pyramidLevelsUsed);
    mvnImgHeight_.resize(pyramidLevelsUsed);
    mvv3fImgGrad_.resize(pyramidLevelsUsed);
    mvfSquareSumGrad_.resize(pyramidLevelsUsed);

    // compute image pyramid before computing gradient
    this->computeImagePyramid_();
}

// default deconstructor
Frame::~Frame() {
    if (mvv3fImgGrad_.size() > 0) {
        for (int level = 0; level < pyramidLevelsUsed; ++level) {
            delete []mvv3fImgGrad_[level];
        }
    }
    if (mvfSquareSumGrad_.size() > 0) {
        for (int level = 0; level < pyramidLevelsUsed; ++level) {
            delete []mvfSquareSumGrad_[level];
        }
    }
}

// display each level image 
void Frame::displayImageLevel(const int& level, std::string outputPath) {
    if (level < 0 || level >= pyramidLevelsUsed) {
        std::cout << "Sorry, failed to display the wrong level image. Please check it again." << std::endl;
        return;
    }

    // construct image 
    cv::Mat image = this->constructImageMat_(level);
    if (image.empty()) {
        std::cout << "Sorry, failed to construct image of cv::Mat, please check it again." << std::endl;
        return;
    }

    // output image 
    // if (outputPath.size() > 0) {
    //     cv::imwrite(outputPath, image);
    // }

    cv::imshow("imageLevel", image);
    cv::waitKey(0);
}

// display all level image
void Frame::displayImageAllLevel() {
    for (int level = 0; level < pyramidLevelsUsed; ++level) {
        std::string path = "./level_";
        path += std::to_string(level);
        path = path + ".png";
        std::cout << "path = " << path << std::endl;
        this->displayImageLevel(level, path);
    }
}

// compute the gradient and square sum of gradient 
// return true if compute successfully 
void Frame::computeGradient_() {

    // make the original image 
    int imgWidth = mvnImgWidth_[0];
    int imgHeight = mvnImgHeight_[0];
    Eigen::Vector3f* pImgGrad = mvv3fImgGrad_[0];
    for (int r = 0; r < imgHeight; ++r) {
        for (int c = 0; c < imgWidth; ++c) {
            pImgGrad[r*imgWidth + c][0] = (float)mmImage_.ptr<uchar>(r)[c];
        }
    }

    // compute gradient of each level of pyramid
    for (int level = 0; level < pyramidLevelsUsed; ++level) {
        int widthLevel = mvnImgWidth_[level];
        int heightLevel = mvnImgHeight_[level];

        Eigen::Vector3f* pImgGradLevel = mvv3fImgGrad_[level];
        float* pAbsSquareGradLevel = mvfSquareSumGrad_[level];
        
        // compute the pixel
        if (level > 0) {
            int levelUp = level - 1;
            int widthLevelUp = mvnImgWidth_[levelUp];
            Eigen::Vector3f* pImgGradLevelUp = mvv3fImgGrad_[levelUp];

            for (int r = 0; r < heightLevel; ++r) {
                for (int c = 0; c < widthLevel; ++c) {
                    pImgGradLevel[r*widthLevel + c][0] = 0.25f * (
                        pImgGradLevelUp[2*c + 2*r*widthLevelUp][0] + 
                        pImgGradLevelUp[2*c+1 + 2*r*widthLevelUp][0] +
                        pImgGradLevelUp[2*c + 2*r*widthLevelUp + widthLevelUp][0] +
                        pImgGradLevelUp[2*c+1 + 2*r*widthLevelUp + widthLevelUp][0] 
                    );
                }
            }
        }

        // compute the gradient
        for (int idx = widthLevel; idx < widthLevel * (heightLevel - 1); ++idx) {
            float dx = 0.5f * (pImgGradLevel[idx + 1][0] - pImgGradLevel[idx - 1][0]);
            float dy = 0.5f * (pImgGradLevel[idx + widthLevel][0] - pImgGradLevel[idx - widthLevel][0]);
            
            // check the valid value
            if (!std::isfinite(dx)) {
                dx = 0;
            }
            if (!std::isfinite(dy)) {
                dy = 0;
            }

            pImgGradLevel[idx][1] = dx;
            pImgGradLevel[idx][2] = dy;
            pAbsSquareGradLevel[idx] = dx * dx + dy * dy;
        }
    }
}

// make image pyramid
void Frame::computeImagePyramid_() {
    // compute image size of each level of pyramid
    int imgWidth = mmImage_.size().width;
    int imgHeight = mmImage_.size().height;
    mvnImgWidth_[0] = imgWidth;
    mvnImgHeight_[0] = imgHeight;
    std::cout << "Each level size of image is : \n";
    std::cout << "level 0 : " << mvnImgWidth_[0] << " X " << mvnImgHeight_[0] << "\n";
    for (int level = 1; level < pyramidLevelsUsed; ++level) {
        mvnImgWidth_[level] = imgWidth >> level;
        mvnImgHeight_[level] = imgHeight >> level;
        std::cout << "level " << level << " : " << mvnImgWidth_[level] << " X " << mvnImgHeight_[level] << "\n";
    }
    std::cout << std::endl;

    // allocate of each level of image pyramid
    for (int level = 0; level < pyramidLevelsUsed; ++level) {
        mvv3fImgGrad_[level] = new Eigen::Vector3f[mvnImgHeight_[level] * mvnImgWidth_[level]];
        mvfSquareSumGrad_[level] = new float[mvnImgHeight_[level] * mvnImgWidth_[level]];
    }

    this->computeGradient_();
}

// construct cv::Mat types
cv::Mat Frame::constructImageMat_(const int& level) {
    int width = mvnImgWidth_[level];
    int height = mvnImgHeight_[level];
    cv::Mat image = cv::Mat(height, width, CV_8UC1);

    Eigen::Vector3f* pImgLevel = mvv3fImgGrad_[level];
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            image.at<uchar>(r, c) = (unsigned char)pImgLevel[r*width + c][0];
        }
    }

    return image;
}