#ifndef SETTING_H
#define SETTING_H

namespace DSOLearn {
    
    // pyramid used 
    #define PYRAMID_LEVELS 6
    extern int pyramidLevelsUsed;

    // parameters controlling pixel selection
    extern float setting_minGradHistCut;
    extern float setting_minGradHistAdd;
    extern float setting_gradDownweightPerLevel;
    extern bool  setting_selectDirectionDistribution;
}


#endif // SETTING_H