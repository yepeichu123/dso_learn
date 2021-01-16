#ifndef PIXEL_SELECTOR_H
#define PIXEL_SELECTOR_H

#include <opencv2/core.hpp>
#include <memory>
#include <eigen3/Eigen/Core>
#include "Frame.h"

namespace DSOLearn {
    
    class PixelSelector {
    public:
        // default constructor
        PixelSelector(const int& width, const int& height);
        
        // default deconstructor
        ~PixelSelector();
        
        // extract pixel and return the total number
        int makeMaps(const std::shared_ptr<Frame>& frame, float* outputMap, const float& density, int recursionLeft = 1);

        // display pixels we selected
        void displayPixelsSelected(const std::shared_ptr<Frame>& frame, float* outputMap);

    private:
        // member function: selector
        Eigen::Vector3i selectPixel_(const std::shared_ptr<Frame>& frame, float* outputMap, const int& pot);

        // member function: make histogram according to the size of pot
        void makeHists_(const std::shared_ptr<Frame>& frame);

        // compute the threshold
        int computeHistQuantil_(int* hist, const float& below);

    private:
        // image size 
        int mnWidth_, mnHeight_;

        // grid width
        int mnGridStep_;

        // histograms of 32*32 grid
        int* mpGradHist_;

        // threshold
        float* mpThreshold_;

        // threshold smoothed
        float* mpThresholdSmoothed_;

        // flag to check whether we build histograms
        bool mbMakeHist_;

        // random pattern used for selecting direction
        unsigned char* mpRandomPattern_;

        // size of potential
        int mnCurrentPotential_;
    };
}

#endif // PIXEL_SELECTOR_H