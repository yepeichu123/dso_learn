#include "PixelSelector.h"
#include <cmath>
#include <stdlib.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace DSOLearn;

// default constructor
PixelSelector::PixelSelector(const int& width, const int& height) {

    mbMakeHist_ = false;
    mpGradHist_ = new int[100 * (1 + width/32)*(1 + height/32)];
    mpThreshold_ = new float[(width/32) * (height/32) + 100];
    mpThresholdSmoothed_ = new float[(width/32) * (height/32) + 100];
    mnCurrentPotential_ = 3;

    mpRandomPattern_ = new unsigned char[width * height];
    std::srand(3141592);
    for (int i = 0; i < width * height; ++i) {
        mpRandomPattern_[i] = std::rand() & 0xFF;
    }
}
        
// default deconstructor
PixelSelector::~PixelSelector() {
    delete []mpGradHist_;
    delete []mpThreshold_;
    delete []mpThresholdSmoothed_;
    delete []mpRandomPattern_;
}

// extract pixel and return the total number
int PixelSelector::makeMaps(const std::shared_ptr<Frame>& frame, float* outputMap, 
    const float& density, int recursionLeft) {

    // ideal number pixels we want
    float idealPixelNum = density;
    // actual number pixels we have
    float havePixelNum = 0;
    float quotia;
    int idealPotential = mnCurrentPotential_;

    // we should make histograms first
    this->makeHists_(frame);

    // select pixels
    Eigen::Vector3i numPixelSelected = this->selectPixel_(frame, outputMap, idealPotential);
    havePixelNum = numPixelSelected[0] + numPixelSelected[1] + numPixelSelected[2];
    quotia = idealPixelNum / havePixelNum;

    // by default we want to over-sample by 40% just to be sure
    float K = havePixelNum * (mnCurrentPotential_+1) * (mnCurrentPotential_+1);
    idealPotential = sqrtf(K/idealPixelNum) - 1;
    if (idealPotential < 1) {
        idealPotential = 1;
    }

    // resample to get more points
    if (recursionLeft > 0 && quotia > 1.25 && mnCurrentPotential_ > 1) {
        if (idealPotential >= mnCurrentPotential_) {
            idealPotential = mnCurrentPotential_ - 1;
        }
        mnCurrentPotential_ = idealPotential;
        return makeMaps(frame, outputMap, density, recursionLeft-1);
    }
    // resample to get less points
    else if (recursionLeft > 0 && quotia < 0.25) {
        if (idealPotential <= mnCurrentPotential_) {
            idealPotential = mnCurrentPotential_ + 1;
        }
        mnCurrentPotential_ = idealPotential;
        return makeMaps(frame, outputMap, density, recursionLeft-1);
    }

    // remove some pixels since we have so much pixels now
    int havePixelNumSub = havePixelNum;
    if (quotia < 0.95) {
        int wh = frame->mvnImgWidth_[0] * frame->mvnImgHeight_[0];
        int rn = 0;
        unsigned char charTh = 255 * quotia;
        for (int i = 0; i < wh; ++i) {
            if (outputMap[i] != 0) {
                if (mpRandomPattern_[rn] > charTh) {
                    outputMap[i] = 0;
                    --havePixelNumSub;
                }
                ++rn;
            }
        }
    }

    mnCurrentPotential_ = idealPotential;

    return havePixelNumSub;
}

// display pixels we selected
void PixelSelector::displayPixelsSelected(const std::shared_ptr<Frame>& frame, float* outputMap) {
    assert(frame != nullptr);

    Eigen::Vector3f* pGradImage = frame->mvv3fImgGrad_[0];
    int width = frame->mvnImgWidth_[0];
    int height = frame->mvnImgHeight_[0];

    cv::Mat imgWithSelectedPixels = frame->mmImage_.clone();
    cv::cvtColor(imgWithSelectedPixels, imgWithSelectedPixels, cv::COLOR_GRAY2RGB);

    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            int idx = w + h*width;
            if (outputMap[idx] == 1) {
                cv::circle(imgWithSelectedPixels, cv::Point2f(w, h), 5, cv::Scalar(0, 255, 0));
            }
            else if (outputMap[idx] == 2) {
                cv::circle(imgWithSelectedPixels, cv::Point2f(w, h), 10, cv::Scalar(0, 0, 255));
            }
            else if (outputMap[idx] == 4) {
                cv::circle(imgWithSelectedPixels, cv::Point2f(w, h), 20, cv::Scalar(255, 0, 0));
            }
        }
    }

    // output image 
    // std::string path = "./imgWithSelectedPixels.png";
    // cv::imwrite(path, imgWithSelectedPixels);

    // display image
    cv::imshow("imgWithSelectedPixels", imgWithSelectedPixels);
    cv::waitKey(0);
}

// member function: selector
Eigen::Vector3i PixelSelector::selectPixel_(const std::shared_ptr<Frame>& frame, float* outputMap, const int& pot) {
    assert(frame != nullptr);

    Eigen::Vector3f* gradImage0 = frame->mvv3fImgGrad_[0];
    // square sum of gradient of the first three levels
    float* squareGradSum0 = frame->mvfSquareSumGrad_[0];
    float* squareGradSum1 = frame->mvfSquareSumGrad_[1];
    float* squareGradSum2 = frame->mvfSquareSumGrad_[2];
    // step of the first three levels
    int height0 = frame->mvnImgHeight_[0];
    int width0 = frame->mvnImgWidth_[0];
    int width1 = frame->mvnImgWidth_[1];
    int widht2 = frame->mvnImgWidth_[2];

    // initialize the value of the output map
    memset(outputMap, 0, width0*height0*sizeof(int));

    // set up down weight of each level
    float downWeight1 = setting_gradDownweightPerLevel;
    float downWeight2 = downWeight1 * downWeight1;

    // setup direction 
    // we do not want that gradient of all pixel is the same direction 
    const Eigen::Vector2f vDirection[16] = {
        Eigen::Vector2f(0,         1.0000),
        Eigen::Vector2f(0.3827,    0.9239),
        Eigen::Vector2f(0.1951,    0.9808),
        Eigen::Vector2f(0.9239,    0.3827),
        Eigen::Vector2f(0.7071,    0.7071),
        Eigen::Vector2f(0.3827,   -0.9239),
        Eigen::Vector2f(0.8315,    0.5556),
        Eigen::Vector2f(0.8315,   -0.5556),
        Eigen::Vector2f(0.5556,   -0.8315),
        Eigen::Vector2f(0.9808,    0.1951),
        Eigen::Vector2f(0.9239,   -0.3827),
        Eigen::Vector2f(0.7071,   -0.7071),
        Eigen::Vector2f(0.5556,    0.8315),
        Eigen::Vector2f(0.9808,   -0.1951),
        Eigen::Vector2f(1.0000,    0.0000),
        Eigen::Vector2f(0.1951,   -0.9808)
    };

    int numInPot = 0, numIn2Pot = 0, numIn4Pot = 0;
    // 4 times pot
    for (int y4 = 0; y4 < height0; y4 += (4*pot)) {
        for (int x4 = 0; x4 < width0; x4 += (4*pot)) {
            int mh3 = std::min((4*pot), height0-y4);
            int mw3 = std::min((4*pot), width0-x4);
            int bestIdx4 = -1;
            float bestVal4 = 0;
            Eigen::Vector2f direct4 = vDirection[mpRandomPattern_[numInPot] & 0xF];

            // 2 times pot
            for (int y3 = 0; y3 < mh3; y3 += (2*pot)) {
                for (int x3 = 0; x3 < mw3; x3 += (2*pot)) {
                    int x34 = x3 + x4;
                    int y34 = y3 + y4;
                    int mh2 = std::min((2*pot), height0-y34);
                    int mw2 = std::min((2*pot), width0-x34);
                    int bestIdx3 = -1;
                    float bestVal3 = 0;
                    Eigen::Vector2f direct3 = vDirection[mpRandomPattern_[numInPot] & 0xF];

                    // 1 times pot
                    for (int y2 = 0; y2 < mh2; y2 += pot) {
                        for (int x2 = 0; x2 < mw2; x2 += pot) {
                            int x234 = x2 + x34;
                            int y234 = y2 + y34;
                            int mh1 = std::min(pot, height0-y234);
                            int mw1 = std::min(pot, width0-x234);
                            int bestIdx2 = -1;
                            float bestVal2 = 0;
                            Eigen::Vector2f direct2 = vDirection[mpRandomPattern_[numInPot] & 0xF];

                            // go throudh each pixel in the pot
                            for (int y1 = 0; y1 < mh1; ++y1) {
                                for (int x1 = 0; x1 < mw1; ++x1) {
                                    assert(x1 + x234 < width0);
                                    assert(y1 + y234 < height0);
                                    int idx = x1+x234 + width0*(y1+y234);
                                    int xf = x1 + x234;
                                    int yf = y1 + y234;
                                    
                                    if (xf < 4 || xf > width0 - 5 || yf < 4 || yf > height0 - 4) {
                                        continue;
                                    }

                                    float pixelThreshold0 = mpThresholdSmoothed_[(xf >> 5) + (yf >> 5)*mnGridStep_];
                                    float pixelThreshold1 = pixelThreshold0 * downWeight1;
                                    float pixelThreshold2 = pixelThreshold0 * downWeight2;

                                    // first level pixel
                                    float gradSum0 = squareGradSum0[idx];
                                    if (gradSum0 > pixelThreshold0) {
                                        Eigen::Vector2f grad0 = gradImage0[idx].tail<2>();
                                        float directNorm = fabsf((float)(grad0.dot(direct2)));
                                        if (!setting_selectDirectionDistribution) {
                                            directNorm = gradSum0;
                                        }

                                        if (directNorm > bestVal2) {
                                            bestVal2 = directNorm;
                                            bestIdx2 = idx;
                                            bestIdx3 = -2;
                                            bestIdx4 = -2;
                                        }
                                    }
                                    // we found the suitable pixel in the first level
                                    // so we should not search pixel in the next level
                                    if (bestIdx3 == -2) {
                                        continue;
                                    }

                                    // we have not found suitable pixel in the first level and go on searching
                                    float gradSum1 = squareGradSum1[int(xf*0.5f + 0.25f) + int(yf*0.5f + 0.25f)*width1];
                                    if (gradSum1 > pixelThreshold1) {
                                        Eigen::Vector2f grad0 = gradImage0[idx].tail<2>();
                                        float directNorm = fabsf((float)(grad0.dot(direct3)));
                                        if (!setting_selectDirectionDistribution) {
                                            directNorm = gradSum1;
                                        }

                                        if (directNorm > bestVal3) {
                                            bestVal3 = directNorm;
                                            bestIdx3 = idx;
                                            bestIdx4 = -2;
                                        }
                                    }
                                    // we found the suitable pixel in the second level
                                    // so we should not search pixel in the next level
                                    if (bestIdx4 == -2) {
                                        continue;
                                    }

                                    // we have not found suitable pixel in the second level an go on searching
                                    float gradSum2 = squareGradSum2[int(xf*0.25f+0.125f) + int(yf*0.25f+0.125f)*widht2];
                                    if (gradSum2 > pixelThreshold2) {
                                        Eigen::Vector2f grad0 = gradImage0[idx].tail<2>();
                                        float directNorm = fabsf((float)(grad0.dot(direct4)));
                                        if (!setting_selectDirectionDistribution) {
                                            directNorm = gradSum2;
                                        }

                                        if (directNorm > bestVal4) {
                                            bestVal4 = directNorm;
                                            bestIdx4 = idx;
                                        }
                                    }
                                }
                            }

                            // check if we found the suitable pixel in the first level
                            if (bestIdx2 > 0) {
                                outputMap[bestIdx2] = 1;
                                bestVal3 = 1e10;
                                ++numInPot;
                            }
                        }
                    }
                    
                    // check if we found the suitable pixel in the second level
                    if (bestIdx3 > 0) {
                        outputMap[bestIdx3] = 2;
                        bestVal4 = 1e10;
                        ++numIn2Pot;
                    }
                }
            }

            // check if we found the suitable pixel in the third level
            if (bestIdx4 > 0) {
                outputMap[bestIdx4] = 4;
                ++numIn4Pot;
            }
        }
    }

    return Eigen::Vector3i(numInPot, numIn2Pot, numIn4Pot);
}

// member function: make histogram according to the size of pot
void PixelSelector::makeHists_(const std::shared_ptr<Frame>& frame) {
    if (mbMakeHist_) {
        return;
    }

    assert(frame != nullptr);
    
    // read data from frame
    float* squareSumGrad = frame->mvfSquareSumGrad_[0];
    int w = frame->mvnImgWidth_[0];
    int h = frame->mvnImgHeight_[0];

    // construct grid of 32 * 32
    int w32 = w / 32;
    int h32 = h / 32;
    mnGridStep_ = w32;

    // go through all grids
    for (int r = 0; r < h32; ++r) {
        for (int c = 0; c < w32; ++c) {
            // start point of grid
            float* squareSumGradGrid = squareSumGrad + 32*c + 32*r*w;

            // calculate the histograms
            for (int i = 0; i < 32; ++i) {
                for (int j = 0; j < 32; ++j) {
                    int xGrid = j + 32 * c;
                    int yGrid = i + 32 * r;

                    if (xGrid < w - 2 || xGrid < 1 ||
                        yGrid < h - 2 || yGrid < 1) {
                            continue;
                    }
                    
                    int grad = sqrtf(squareSumGradGrid[j + i*w]);
                    // threshold defined by DSO
                    if (grad > 48) {
                        grad = 48;
                    }

                    // count histograms
                    mpGradHist_[grad + 1]++;
                    mpGradHist_[0]++;
                }
            }
            // compute threshold
            mpThreshold_[c + r*w32] = this->computeHistQuantil_(mpGradHist_, setting_minGradHistCut) + setting_minGradHistAdd;
        }
    }
    
    // compute the smoothed threshold
    for (int r = 0; r < h32; ++r) {
        for (int c = 0; c < w32; ++c) {
            float sum = 0, num = 0;
            // left 3 grids
            if (c > 0) {
                if (r > 0) {
                    ++num;
                    sum += mpThreshold_[c-1 + (r-1)*w32];
                }
                if (r < h32 - 1) {
                    ++num;
                    sum += mpThreshold_[c-1 + (r+1)*w32];
                }
                ++num;
                sum += mpThreshold_[c-1 + r*w32];
            }
            // right 3 grids
            if (c < w32 - 1) {
                if (r > 0) {
                    ++num;
                    sum += mpThreshold_[c+1 + (r-1)*w32];
                }
                if (r < w32 - 1) {
                    ++num;
                    sum += mpThreshold_[c+1 + (r+1)*w32];
                }
                ++num;
                sum += mpThreshold_[c+1 + r*w32];
            }
            // top
            if (r > 0) {
                ++num;
                sum += mpThreshold_[c + (r-1)*w32];
            }
            // bottom
            if (r < h32 - 1) {
                ++num;
                sum += mpThreshold_[c + (r+1)*w32];
            }
            // center
            ++num;
            sum += mpThreshold_[c + r*w32];

            // used for selecting pixels
            mpThresholdSmoothed_[c + r*w32] = (sum / num) * (sum / num);
        }
    }

    mbMakeHist_ = true;
}

// compute the threshold
// actually it looks like the medium value
int PixelSelector::computeHistQuantil_(int* hist, const float& below) {
    int threshold = hist[0]*below + 0.5f;
    for (int i = 0; i < 90; ++i) {
        threshold -= hist[i + 1];
        if (threshold < 0) {
            return i;
        }
    }
    return 90;
}