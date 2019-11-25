#pragma once

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "detectorBase.hpp"

using namespace std;
using namespace cv;

class FastFeature: public DetectorBase {
public:
    FastFeature(int width, int height, int cellSize, int levels, int threshold);

    bool detect(vector<Mat>& images, vector<MKeyPoint>& keyPoints);

private:
    int mThreshold;
};
