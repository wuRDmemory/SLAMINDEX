#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

#include "detectorBase.hpp"

using namespace std;
using namespace cv;

class orbFeature : public DetectorBase {
public:
    orbFeature(int width, int height, int cellSize, int levels, int ftrNum, int threshold);

    bool detect(vector<Mat>& images, vector<MKeyPoint>& keyPoints);

private:
    bool distributeOctTree(vector<KeyPoint>& distKeyPoint, Point tl, Point br, int predKPCnt);

private:
    int mThreshold;
    int mMaxFtrNum;

    vector<int>  mLevelFeatureCnt;
    vector<Rect> mROIs;
};
