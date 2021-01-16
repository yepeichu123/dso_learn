#include "Setting.h"


namespace DSOLearn {

    // pyramid used 
    int pyramidLevelsUsed = PYRAMID_LEVELS;
    
    // parameters controlling pixel selection
    float setting_minGradHistCut = 0.5;
    float setting_minGradHistAdd = 7;
    float setting_gradDownweightPerLevel = 0.75;
    bool  setting_selectDirectionDistribution = true;
}

