#ifndef FRAME_H
#define FRAME_H

#include <opencv2/core.hpp>
#include <eigen3/Eigen/Core>
#include <vector>
#include <string>
#include <memory>
#include "Setting.h"

namespace DSOLearn {

    class Frame {
    public:
        // default constructor
        Frame(const cv::Mat& image);

        // default deconstructor
        ~Frame();

        // display each level image 
        void displayImageLevel(const int& level, std::string outputPath = "");

        // display all level image
        void displayImageAllLevel();

    private:
        // compute the gradient and square sum of gradient 
        void computeGradient_();

        // make image pyramid
        void computeImagePyramid_();

        // construct cv::Mat types
        cv::Mat constructImageMat_(const int& level);

    public:
        // image 
        cv::Mat mmImage_;

        // image size 
        std::vector<int> mvnImgWidth_, mvnImgHeight_;

        // first dimension is image pixel
        // the last two dimensions are gradient of each pixel 
        std::vector<Eigen::Vector3f*> mvv3fImgGrad_;

        // square sum of gradient of image 
        std::vector<float*> mvfSquareSumGrad_;
    };
}

#endif // FRAME_H