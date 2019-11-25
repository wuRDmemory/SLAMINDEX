#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

#include "detectorBase.hpp"

using namespace std;
using namespace cv;

class GoodFeatureDist: public DetectorBase {
private:
    vector<Rect> mROIs;
    vector<bool> mOccupied;

public:
    GoodFeatureDist(int width, int height, int cellSize, int levels);
    ~GoodFeatureDist();


    /** images   : image pyramid     
     ** keyPoints: result key points 
     ** levels   : key points levels
     **/
    virtual bool detect(vector<Mat>& images, vector<MKeyPoint>& keyPoints);
};