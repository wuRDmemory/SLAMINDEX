#include "orbFeature.hpp"

orbFeature::orbFeature(int width, int height, int cellSize, int levels, int ftrNum, int threshold):
    DetectorBase(width, height, cellSize, levels), mThreshold(threshold), mMaxFtrNum(ftrNum) {
    /*   calculate each level key point number   */
    mLevelFeatureCnt.resize(levels);

    const float invFactor = 0.5f;
    mLevelFeatureCnt[0]   = int(ftrNum*(1 - 0.5f)/(1 - pow(invFactor, levels)));
    for (int i = 1; i < mLevels; i++) {
        mLevelFeatureCnt[i] = invFactor*mLevelFeatureCnt[i-1];
    }
}

bool orbFeature::detect(vector<Mat>& images, vector<MKeyPoint>& keyPoints) {
    assert(images.size() == mLevels);

    /*  fetch all potential point in each level  */
    for (int k = 0; k < mLevels; k++) {
        const int w = images[k].cols;
        const int h = images[k].rows;

        const int nW = w/mCellSize;
        const int nH = h/mCellSize;

        vector<KeyPoint> distKeyPoints;
        distKeyPoints.reserve(mMaxFtrNum*10);

        for (int i = 0; i < nH; i++)
        for (int j = 0; j < nW; j++) {
            Point tl((i + 0)*nW, (i + 0)*nH - 1);
            Point br((i + 1)*nW, (i + 1)*nH - 1);
            
            vector<KeyPoint> kps;
            FAST(images[k].colRange(tl.x, br.x).rowRange(tl.y, br.y), kps, mThreshold);

            for (int i = 0; i < kps.size(); i++) {
                distKeyPoints.push_back(kps[i]);
            }
        }

        /*   distribute them  */
        distributeOctTree(distKeyPoints, Point(0, 0), Point(w, h), mLevelFeatureCnt[k]);
    }
}

bool orbFeature::distributeOctTree(vector<KeyPoint>& distKeyPoint, Point tl, Point br, int predKPCnt) {
    int initSeq = int((br.x - tl.x)/(br.y - tl.y));
    
}