#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

#include "detectorBase.hpp"

using namespace std;
using namespace cv;

struct OctNode {
    bool mNoMore;
    Rect mRect;
    vector<KeyPoint> mPts;

    OctNode() : mNoMore(false) {}
    bool split(OctNode& n1, OctNode& n2, OctNode& n3, OctNode& n4);
};

class ORBFeature : public DetectorBase {
public:
    ORBFeature(int width, int height, int cellSize, int levels, int ftrNum, int threshold);

    bool detect(vector<Mat>& images, vector<MKeyPoint>& keyPoints);

private:
    bool distributeOctTree(Mat& image, vector<KeyPoint>& distKeyPoint, vector<Point2f>& keyPoints, Point tl, Point br, int predKPCnt);

private:
    int mThreshold;
    int mMaxFtrNum;

    vector<int>  mLevelFeatureCnt;
    vector<Rect> mROIs;
};
