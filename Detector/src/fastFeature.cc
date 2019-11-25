#include "fastFeature.hpp"

FastFeature::FastFeature(int width, int height, int cellSize, int levels, int threshold): 
    DetectorBase(width, height, cellSize, levels), mThreshold(threshold) {
    ;
}

bool FastFeature::detect(vector<Mat>& images, vector<MKeyPoint>& keyPoints) {
    int nLevel = images.size();
    /*  search in each level  */
    for (int i = 0; i < nLevel; i++) {
        vector<KeyPoint> kps;
        FAST(images[i], kps, mThreshold);

        /*  find the most strong response key point  */
        for (int j = 0; j < kps.size(); j++) {
            KeyPoint& kp = kps[j];
            Point2f   pt = kp.pt;
            if (pt.x < mBorder          || pt.y < mBorder          || 
                pt.x > mWidth - mBorder || pt.y > mHeight - mBorder) {
                continue;
            }

            const int x  = pt.x / mCellSize;
            const int y  = pt.y / mCellSize;
            if (mCells[y*mCols + x].response < kp.response) {
                mCells[y*mCols + x].response = kp.response;
                mCells[y*mCols + x].px    = pt*(1<<i);
                mCells[y*mCols + x].level = i;
            }
        }
    }

    /*  build result and give a summary  */
    vector<int> smryLevelCnt(nLevel, 0);
    keyPoints.reserve(mCols*mRows);
    for (int i = 0; i < mCols*mRows; i++) {
        Cell& cell = mCells[i];
        if (cell.level < 0)
            continue;
        keyPoints.push_back(MKeyPoint(cell.px, 0, cell.response, cell.level));
        smryLevelCnt[cell.level]++;
    }

    for (int i = 0; i < nLevel; i++) {
        cout << "Level " << i << " detect " << smryLevelCnt[i] << " Points" << endl;
    }
    return true;
}
